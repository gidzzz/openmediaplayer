#ifndef MEDIAART_H
#define MEDIAART_H

#include <hildon-thumbnail/hildon-albumart-factory.h>
#include <hildon-thumbnail/hildon-thumbnail-factory.h>

#include "includes.h"

namespace MediaArt
{
    struct destructor_payload
    {
        HildonThumbnailFactory *factory;
        HildonThumbnailRequest *request;
    };

    // Returns: path to the new image
    QString setAlbumImage(QString album, QString image);
    void destructor(gpointer user_data);

    QString albumArtPath(QString album);
}

#endif // MEDIAART_H
