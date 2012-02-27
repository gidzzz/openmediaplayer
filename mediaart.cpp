#include "mediaart.h"

QString MediaArt::setAlbumImage(QString album, QString image)
{
    gchar* file;
    gchar* uri;

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
    uri = g_filename_to_uri(newArtFile.toUtf8(), NULL, NULL);
    file = hildon_thumbnail_get_uri(uri, 124, 124, true);
    QString newThumbFile = QString::fromUtf8(file).remove("file://");
    delete file;

    // Remove old thumbnail
    if (image.isEmpty() && QFileInfo(newThumbFile).exists())
        QFile::remove(newThumbFile);

    // Generate new thumbanil
    if (!image.isEmpty()) {
        destructor_payload *payload = new destructor_payload;
        payload->factory = hildon_thumbnail_factory_get_instance();
        payload->request = hildon_thumbnail_factory_request_uri(payload->factory, uri,
                                                                124, 124, true, "image/jpeg",
                                                                NULL, payload, destructor);
    }

    delete uri;

    return image.isEmpty() ? albumImage : newArtFile;
}

void MediaArt::destructor(gpointer user_data)
{
    destructor_payload *payload = static_cast<destructor_payload*>(user_data);
    g_object_unref(payload->request);
    g_object_unref(payload->factory);
    delete payload;
}

QString MediaArt::albumArtPath(QString album)
{
    gchar *path = hildon_albumart_get_path(NULL, album.toUtf8(), "album");
    QString path_qstr = QString::fromUtf8(path);
    delete path;
    return path_qstr;
}
