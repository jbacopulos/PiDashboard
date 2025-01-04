#include "XmlReader.h"
#include <QXmlStreamReader>
#include <QDebug>

XmlReader::XmlReader(QObject *parent)
    : QObject(parent),
    networkManager(new QNetworkAccessManager(this))
{
    // Connect the finished signal of the network manager to our slot
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &XmlReader::onReplyFinished);
}

XmlReader::~XmlReader()
{
    // QNetworkAccessManager is deleted automatically because of QObject parent
}

void XmlReader::loadFeed(const QUrl &url, const XmlType &newType)
{
    if (!url.isValid()) {
        emit errorOccurred("Invalid URL.");
        return;
    }

    type = newType;
    QNetworkRequest request(url);
    networkManager->get(request);
}

QList<XmlItem> XmlReader::getItems() const
{
    return items;
}

void XmlReader::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    parseFeed(reply);
    reply->deleteLater();
}

void XmlReader::parseFeed(QIODevice *device)
{
    items.clear();

    QXmlStreamReader xml(device);
    bool inItem = false;
    NewsItem newsItem;
    ForecastItem forecastItem;

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartElement) {
            QString name = xml.name().toString();

            if (name == XmlTag.at(XmlTagEnum::NEWS_START) || (name == XmlTag.at(XmlTagEnum::WEATHER_START) && type == XmlType::WEATHER) || name == XmlTag.at(XmlTagEnum::FORECAST_START)) {
                inItem = true;
            } else if (inItem) {
                if (type == XmlType::NEWS) {
                    if (name == "title") newsItem.title = xml.readElementText();
                    else if (name == "link") newsItem.link = xml.readElementText();
                    else if (name == "description") newsItem.description = xml.readElementText();
                    else if (name == "pubDate") newsItem.pubDate = xml.readElementText();
                } else if (type == XmlType::WEATHER && name == "temperature") {
                    TemperatureItem tempItem;
                    QXmlStreamAttributes attributes = xml.attributes();
                    tempItem.temperature = attributes.value("value").toString();
                    items.append(tempItem);
                } else if (type == XmlType::WEATHER && name == "feels_like") {
                    FeelsLikeItem feelsItem;
                    QXmlStreamAttributes attributes = xml.attributes();
                    feelsItem.feelsLike = attributes.value("value").toString();
                    items.append(feelsItem);
                } else if (type == XmlType::WEATHER && name == "weather") {
                    WeatherItem weatherItem;
                    QXmlStreamAttributes attributes = xml.attributes();
                    weatherItem.weather = attributes.value("value").toString();
                    items.append(weatherItem);
                } else if (type == XmlType::FORECAST) {
                    if (name == "maxtemp_c") forecastItem.maxTemp = xml.readElementText();
                    else if (name == "mintemp_c") forecastItem.minTemp = xml.readElementText();
                    else if (name == "date") forecastItem.date = xml.readElementText();
                    else if (name == "daily_chance_of_rain") forecastItem.rainChance = xml.readElementText();
                    else if (name == "icon") forecastItem.icon = xml.readElementText();
                }
            }
        } else if (token == QXmlStreamReader::EndElement) {
            QString name = xml.name().toString();

            if ((name == XmlTag.at(XmlTagEnum::NEWS_END) || (name == XmlTag.at(XmlTagEnum::WEATHER_END) && type == XmlType::WEATHER) || name == XmlTag.at(XmlTagEnum::FORECAST_END)) && inItem) {
                inItem = false;
                if (name == XmlTag.at(XmlTagEnum::NEWS_END)) {
                    items.append(newsItem);
                } else if (name == XmlTag.at(XmlTagEnum::FORECAST_END)) {
                    items.append(forecastItem);
                }
            }
        }
    }

    if (xml.hasError()) {
        emit errorOccurred("XML Parsing Error: " + xml.errorString());
        return;
    }

    emit feedLoaded();
}
