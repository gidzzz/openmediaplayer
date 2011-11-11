#ifndef MEDIAART_H
#define MEDIAART_H

#include <hildon-thumbnail/hildon-albumart-factory.h>
#include <hildon-thumbnail/hildon-thumbnail-factory.h>

#include "includes.h"

namespace MediaArt
{
    // Returns: path to the new image
    QString setAlbumImage(QString album, QString image);

    QString albumArtPath(QString album);
};

#endif // MEDIAART_H
