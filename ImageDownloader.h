#ifndef IMAGEDOWNLOADER_H
#define IMAGEDOWNLOADER_H

#include <QObject>
#include <QPixmap>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class ImageDownloader : public QObject
{
    Q_OBJECT

public:
    explicit ImageDownloader(QObject *parent = nullptr);
    ~ImageDownloader();

    // Initiates the download of an image from the given URL
    void downloadImage(const QString &url);

    // Retrieves the downloaded pixmap
    QPixmap getPixmap() const;

signals:
    // Signal emitted when the image is downloaded
    void imageDownloaded();

private slots:
    // Slot to handle the finished download
    void onDownloadFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkManager;
    QPixmap pixmap;
};

#endif // IMAGEDOWNLOADER_H
