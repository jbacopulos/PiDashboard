#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QFontDatabase>
#include <QTimer>
#include <QDateTime>
#include <QPalette>
#include <QLocale>
#include <QLabel>
#include <QCoreApplication>
#include <QUrl>
#include <algorithm>
#include <QFile>

#include "ImageDownloader.h"
#include "XmlReader.h"

namespace {
bool compareByDate(const ForecastItem &a, const ForecastItem &b) {
    return a.date < b.date;
}

QString roundQString(const QString &str) {
    return QString::number(qRound(str.toFloat()));
}

void clearLayout(QLayout *layout) {
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 480;
static const int NEWS_MAX_ITEMS = 8;
static const int ICON_SIZE = 30;

static const QUrl NEWS_URL("https://feeds.bbci.co.uk/news/world/rss.xml");
static QUrl WEATHER_URL;
static QUrl FORECAST_URL;
}

MainWindow::MainWindow(const std::map<QString, QString> &envVars, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , newsReader(this)
    , weatherReader(this)
    , forecastReader(this)
    , downloaderDay1(this)
    , downloaderDay2(this)
    , downloaderDay3(this)
{
    WEATHER_URL = QUrl(QString("https://api.openweathermap.org/data/2.5/weather?zip=%1&mode=xml&units=%2&appid=%3").arg(
        envVars.at("ZIP"),
        envVars.at("UNIT"),
        envVars.at("OW_API_KEY")
    ));
    FORECAST_URL = QUrl(QString("https://api.weatherapi.com/v1/forecast.xml?key=%1&q=%2&days=3").arg(
        envVars.at("W_API_KEY"),
        envVars.at("ZIP").split(",")[0]
    ));

    ui->setupUi(this);
    setupWindow();
    setupBackground();
    setupFonts();
    setupTimers();

    // Connect newsReader to onNewsLoaded
    connect(&newsReader, &XmlReader::feedLoaded, this, &MainWindow::onNewsLoaded);
    connect(&newsReader, &XmlReader::errorOccurred, this, &MainWindow::onErrorQuit);

    newsReader.loadFeed(NEWS_URL, XmlType::NEWS);

    // Connect weatherReader to onWeatherLoaded
    connect(&weatherReader, &XmlReader::feedLoaded, this, &MainWindow::onWeatherLoaded);
    connect(&weatherReader, &XmlReader::errorOccurred, this, &MainWindow::onErrorQuit);

    weatherReader.loadFeed(WEATHER_URL, XmlType::WEATHER);

    // Connect forecastReader to onForecastLoaded
    connect(&forecastReader, &XmlReader::feedLoaded, this, &MainWindow::onForecastLoaded);
    connect(&forecastReader, &XmlReader::errorOccurred, this, &MainWindow::onErrorQuit);

    forecastReader.loadFeed(FORECAST_URL, XmlType::FORECAST);

    // Download weather icons
    connect(&downloaderDay1, &ImageDownloader::imageDownloaded, this, [this]() {
        ui->day1icon->setPixmap(
            downloaderDay1.getPixmap().scaled(ICON_SIZE, ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            );
    });

    connect(&downloaderDay2, &ImageDownloader::imageDownloaded, this, [this]() {
        ui->day2icon->setPixmap(
            downloaderDay2.getPixmap().scaled(ICON_SIZE, ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            );
    });

    connect(&downloaderDay3, &ImageDownloader::imageDownloaded, this, [this]() {
        ui->day3icon->setPixmap(
            downloaderDay3.getPixmap().scaled(ICON_SIZE, ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            );
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupWindow()
{
    this->setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    this->setWindowFlag(Qt::FramelessWindowHint);
}

void MainWindow::setupBackground()
{
    QPixmap background(":/background.jpg");
    background = background.scaled(this->size(), Qt::IgnoreAspectRatio);
    QPalette palette;
    palette.setBrush(QPalette::Window, background);
    this->setPalette(palette);
}

void MainWindow::setupFonts()
{
    int fontId = QFontDatabase::addApplicationFont(":/Roboto-Round.ttf");
    if (fontId < 0) {
        qWarning() << "Failed to load Roboto-Round.ttf!";
        return;
    }

    QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);

    // Large time
    QFont timeFontLg(fontFamily, 72);
    ui->timeText->setFont(timeFontLg);

    // Seconds & AM/PM
    QFont timeFontSm(fontFamily, 24);
    ui->secondsText->setFont(timeFontSm);
    ui->periodText->setFont(timeFontSm);

    // Date
    QFont dateFont(fontFamily, 18);
    ui->dateText->setFont(dateFont);
    ui->dateText->setText(QDateTime::currentDateTime().toString("dddd, MMMM dd"));

    // Current weather
    QFont weatherFontLg(fontFamily, 48);
    ui->currentTempText->setFont(weatherFontLg);

    QFont weatherFontSm(fontFamily, 18);
    ui->feelsLikeText->setFont(weatherFontSm);
    ui->weatherText->setFont(weatherFontSm);
}

void MainWindow::setupTimers()
{
    QTimer *timer1s = new QTimer(this);
    connect(timer1s, &QTimer::timeout, this, [this]() {
        updateDateTimeDisplay();
    });
    timer1s->start(1000);

    QTimer *timer10m = new QTimer(this);
    connect(timer10m, &QTimer::timeout, this, [this]() {
        newsReader.loadFeed(NEWS_URL, XmlType::NEWS);
        weatherReader.loadFeed(WEATHER_URL, XmlType::WEATHER);
        forecastReader.loadFeed(FORECAST_URL, XmlType::FORECAST);

    });
    timer10m->start(10 * 60 * 1000);

    // Initialize once
    updateDateTimeDisplay();
}

void MainWindow::updateDateTimeDisplay()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    int hour = currentDateTime.time().hour();
    QString hour12 = QString::number((hour % 12 == 0) ? 12 : hour % 12);
    ui->timeText->setText(hour12 + ":" + currentDateTime.time().toString("mm"));
    ui->secondsText->setText(currentDateTime.time().toString("ss"));
    ui->dateText->setText(currentDateTime.toString("dddd, MMMM dd"));
}

void MainWindow::onNewsLoaded()
{
    auto items = newsReader.getItems();
    int maxItems = qMin(items.size(), NEWS_MAX_ITEMS);
    clearLayout(ui->column2->layout());

    for (int i = 0; i < maxItems; ++i) {
        if (!std::holds_alternative<NewsItem>(items[i])) {
            continue;
        }

        const auto &newsItem = std::get<NewsItem>(items[i]);
        QLabel *label = new QLabel(newsItem.title, ui->column2);
        label->setStyleSheet(
            "QFrame {"
            "   background-color: rgba(0, 0, 0, 0);"
            "   border-radius: 0px;"
            "}"
            );
        label->setWordWrap(true);
        ui->column2->layout()->addWidget(label);
    }
}

void MainWindow::onWeatherLoaded()
{
    auto items = weatherReader.getItems();

    // For each variant in items: TemperatureItem, FeelsLikeItem, WeatherItem
    for (auto &item : items) {
        if (std::holds_alternative<TemperatureItem>(item)) {
            QString temperature = std::get<TemperatureItem>(item).temperature;
            ui->currentTempText->setText(roundQString(temperature) + "°");
        }
        else if (std::holds_alternative<FeelsLikeItem>(item)) {
            QString feelsLike = std::get<FeelsLikeItem>(item).feelsLike;
            ui->feelsLikeText->setText("Feels like " + roundQString(feelsLike) + "°");
        }
        else if (std::holds_alternative<WeatherItem>(item)) {
            QString weather = std::get<WeatherItem>(item).weather;
            weather[0] = weather[0].toUpper();
            ui->weatherText->setText(weather);
        }
    }
}

void MainWindow::onForecastLoaded()
{
    const QLocale locale;
    auto items = forecastReader.getItems();

    QList<ForecastItem> forecastList;
    forecastList.reserve(items.size());
    for (auto &variantItem : items) {
        if (std::holds_alternative<ForecastItem>(variantItem)) {
            forecastList.append(std::get<ForecastItem>(variantItem));
        }
    }

    // Sort the forecast items
    std::sort(forecastList.begin(), forecastList.end(), compareByDate);

    if (forecastList.size() < 3) {
        qWarning() << "Not enough forecast data; expected at least 3 days.";
        return;
    }

    // Day 1
    updateForecastUI(forecastList[0], ui->day1hi, ui->day1lo, ui->day1prec, ui->day1icon, downloaderDay1, locale, nullptr);

    // Day 2
    updateForecastUI(forecastList[1], ui->day2hi, ui->day2lo, ui->day2prec, ui->day2icon, downloaderDay2, locale, ui->day2Text);

    // Day 3
    updateForecastUI(forecastList[2], ui->day3hi, ui->day3lo, ui->day3prec, ui->day3icon, downloaderDay3, locale, ui->day3Text);
}

void MainWindow::updateForecastUI(
    const ForecastItem &item,
    QLabel *hiLabel,
    QLabel *loLabel,
    QLabel *precLabel,
    QLabel *iconLabel,
    ImageDownloader &downloader,
    const QLocale &locale,
    QLabel *dayLabel)
{
    hiLabel->setText(roundQString(item.maxTemp) + "°");
    loLabel->setText(roundQString(item.minTemp) + "°");
    precLabel->setText(item.rainChance + "%");

    // If we also have a label to display day name
    if (dayLabel) {
        QDate date = QDate::fromString(item.date, "yyyy-MM-dd");
        dayLabel->setText(locale.dayName(date.dayOfWeek(), QLocale::ShortFormat) + ".");
    }

    downloader.downloadImage("https:" + item.icon);
}

void MainWindow::onErrorQuit(const QString &error)
{
    qCritical() << "Error:" << error;
    QApplication::quit();
}
