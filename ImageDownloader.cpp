#include "ImageDownloader.h"
#include <QNetworkRequest>
#include <QUrl>

ImageDownloader::ImageDownloader(QObject *parent)
    : QObject(parent),
    networkManager(new QNetworkAccessManager(this))
{
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &ImageDownloader::onDownloadFinished);
}

ImageDownloader::~ImageDownloader()
{
    delete networkManager;
}

void ImageDownloader::downloadImage(const QString &url)
{
    QNetworkRequest request(url);
    networkManager->get(request);
}

void ImageDownloader::onDownloadFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray imageData = reply->readAll();
        pixmap.loadFromData(imageData);
        emit imageDownloaded();
    }
    reply->deleteLater();
}

QPixmap ImageDownloader::getPixmap() const
{
    return pixmap;
}
