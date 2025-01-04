#ifndef XMLREADER_H
#define XMLREADER_H

#include <QObject>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>

// Structures to hold XML data
struct NewsItem {
    QString title;
    QString link;
    QString description;
    QString pubDate;
};

struct TemperatureItem {
    QString temperature;
};

struct FeelsLikeItem {
    QString feelsLike;
};

struct WeatherItem {
    QString weather;
};

struct ForecastItem {
    QString date;
    QString maxTemp;
    QString minTemp;
    QString rainChance;
    QString icon;
};

using XmlItem = std::variant<NewsItem, TemperatureItem, FeelsLikeItem, WeatherItem, ForecastItem>;

enum XmlType {
    NEWS,
    WEATHER,
    FORECAST
};

enum XmlTagEnum {
    NEWS_START,
    NEWS_END,
    WEATHER_START,
    WEATHER_END,
    FORECAST_START,
    FORECAST_END
};

static std::map<XmlTagEnum, const char *> XmlTag = {
    {XmlTagEnum::NEWS_START, "item"},
    {XmlTagEnum::NEWS_END, "item"},
    {XmlTagEnum::WEATHER_START, "current"},
    {XmlTagEnum::WEATHER_END, "current"},
    {XmlTagEnum::FORECAST_START, "forecastday"},
    {XmlTagEnum::FORECAST_END, "day"}
};

// XmlReader class definition
class XmlReader : public QObject
{
    Q_OBJECT
public:
    explicit XmlReader(QObject *parent = nullptr);
    ~XmlReader();

    // Method to load XML feed from a given URL
    void loadFeed(const QUrl &url, const XmlType &newType);

    // Getter for the list of XML items
    QList<XmlItem> getItems() const;

signals:
    // Emitted when the feed is successfully loaded and parsed
    void feedLoaded();

    // Emitted when an error occurs
    void errorOccurred(const QString &errorString);

private slots:
    // Slot to handle the network reply
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkManager;
    QList<XmlItem> items;
    XmlType type;

    // Helper method to parse the XML feed
    void parseFeed(QIODevice *device);
};

#endif // XMLREADER_H
