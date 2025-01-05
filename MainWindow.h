#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QKeyEvent>
#include <QLabel>
#include "XmlReader.h"
#include "ImageDownloader.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const std::map<QString, QString> &envVars, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // Overrides the keyPressEvent so that pressing q quits
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Q) {
            QApplication::quit();
        } else {
            QMainWindow::keyPressEvent(event);
        }
    }

private:
    Ui::MainWindow *ui;
    XmlReader newsReader;
    XmlReader weatherReader;
    XmlReader forecastReader;
    ImageDownloader downloaderDay1;
    ImageDownloader downloaderDay2;
    ImageDownloader downloaderDay3;
    QString fontFamily;

    void setupWindow();
    void setupBackground();
    void setupFonts();
    void setupTimers();
    void updateDateTimeDisplay();

    void onNewsLoaded();
    void onWeatherLoaded();
    void onForecastLoaded();
    void onErrorQuit(const QString &error);

    void updateForecastUI(
        const ForecastItem &item,
        QLabel *hiLabel,
        QLabel *loLabel,
        QLabel *precLabel,
        QLabel *iconLabel,
        ImageDownloader &downloader,
        const QLocale &locale,
        QLabel *dayLabel
    );
};
#endif // MAINWINDOW_H
