#include "mediaart.h"

QString MediaArt::setAlbumImage(QString album, QString image)
{
    gchar* file;

    // Get album art path
    file = hildon_albumart_get_path(NULL, album.toUtf8(), "album");
    QString newArtFile = QString::fromUtf8(file);
    delete file;

    // Remove old album art
    if (QFileInfo(newArtFile).exists())
        QFile::remove(newArtFile);

    // Store new album art
    if (!image.isEmpty())
        QFile::copy(image, newArtFile);

    // Get thumbnail path
    QString newArtUri = "file://" + newArtFile;
    file = hildon_thumbnail_get_uri(newArtUri.toUtf8(), 124, 124, true);
    QString newThumbFile = QString::fromUtf8(file);
    delete file;

    // Remove old thumbnail
    if (QFileInfo(newThumbFile).exists())
        QFile::remove(newThumbFile);

    // Generate new thumbanil
    if (!image.isEmpty()) {
        HildonThumbnailFactory* factory = hildon_thumbnail_factory_get_instance();
        HildonThumbnailRequest* request = hildon_thumbnail_factory_request_uri(factory, newArtUri.toUtf8(), 124, 124, true, "image/jpeg", NULL, NULL, NULL);

        hildon_thumbnail_factory_join(factory);
        g_object_unref(request);
        g_object_unref(factory);

        return newArtFile;
    }
    else return albumImage;
}
