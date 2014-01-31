#include "metadatawatcher.h"

MetadataWatcher::MetadataWatcher(MafwAdapterFactory *factory) :
    mafwRenderer(factory->getRenderer()),
    mafwSource(factory->getTempSource()),
    mafwTrackerSource(factory->getTrackerSource()),
    sourceMetadataPresent(false)
{
    // Initialization
    connect(mafwRenderer, SIGNAL(rendererReady()), mafwRenderer, SLOT(getStatus()));
    connect(mafwRenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));

    // Metadata
    connect(mafwRenderer, SIGNAL(metadataChanged(QString,QVariant)),
            this, SLOT(onRendererMetadataChanged(QString,QVariant)));
    connect(mafwRenderer, SIGNAL(signalGetCurrentMetadata(GHashTable*,QString,QString)),
            this, SLOT(onRendererMetadataReceived(GHashTable*,QString,QString)));
    connect(mafwSource, SIGNAL(signalMetadataResult(QString,GHashTable*,QString)),
            this, SLOT(onSourceMetadataReceived(QString,GHashTable*,QString)));
    // Currently this is solely for the purpose of updating the paused position,
    // but it might be a good idea to keep all metadata in sync.
    connect(mafwTrackerSource, SIGNAL(metadataChanged(QString)),
            this, SLOT(onSourceMetadataChanged(QString)));
}

QMap<QString,QVariant> MetadataWatcher::metadata()
{
    return currentMetadata;
}

void MetadataWatcher::setMetadataFromRenderer(QString key, QVariant value)
{
    if (!sourceMetadataPresent)
        backupMetadata[key] = value;

    QVariant &currentValue = currentMetadata[key];
    if (currentValue != value) {
        currentValue = value;
        emit metadataChanged(key, value);
    }
}

void MetadataWatcher::setMetadataFromSource(QString key, QVariant value)
{
    if (sourceMetadataPresent) {
        // Consider source metadata less important than renderer metadata,
        // that is do not overwrite it.
        QVariant &currentValue = currentMetadata[key];
        if (currentValue.isNull()) {
            currentValue = value;
            emit metadataChanged(key, value);
        }
    } else {
        QVariant &currentValue = backupMetadata[key];
        if (currentValue.isNull())
            currentValue = value;
    }
}

void MetadataWatcher::onStatusReceived(MafwPlaylist *, uint index, MafwPlayState, const char *objectId, QString)
{
    disconnect(mafwRenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
               this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));

    connect(mafwRenderer, SIGNAL(mediaChanged(int,char*)), this, SLOT(onMediaChanged(int,char*)));

    onMediaChanged(index, (char *) objectId);
}

void MetadataWatcher::onMediaChanged(int, char *objectId)
{
    qDebug() << "Media changed" << objectId;

    currentObjectId = QString::fromUtf8(objectId);

    backupMetadata.clear();
    sourceMetadataPresent = false;

    // Reset the URI as soon as possible to avoid album art misdetection
    if (currentMetadata.remove(MAFW_METADATA_KEY_URI))
        emit metadataChanged(MAFW_METADATA_KEY_URI, QVariant());

    mafwRenderer->getCurrentMetadata();

    mafwSource->setSource(mafwSource->getSourceByUUID(currentObjectId.left(currentObjectId.indexOf("::"))));
    mafwSource->getMetadata(currentObjectId.toUtf8(), MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                       MAFW_METADATA_KEY_ARTIST,
                                                                       MAFW_METADATA_KEY_ALBUM,

                                                                       MAFW_METADATA_KEY_DURATION,
                                                                       MAFW_METADATA_KEY_IS_SEEKABLE,

                                                                       MAFW_METADATA_KEY_URI,
                                                                       MAFW_METADATA_KEY_MIME,

                                                                       MAFW_METADATA_KEY_ALBUM_ART_URI,

                                                                       MAFW_METADATA_KEY_PAUSED_POSITION));
}

void MetadataWatcher::onSourceMetadataReceived(QString objectId, GHashTable *metadata, QString)
{
    if (objectId != currentObjectId) return;

    qDebug() << "Source metadata received";

    GList *keys = g_hash_table_get_keys(metadata);

    for (GList *key = keys; key; key = key->next) {
        QString keyName((const char *) key->data);

        // Paused position might require some processing to prevent confusion in
        // the video window.
        if (keyName == MAFW_METADATA_KEY_PAUSED_POSITION) {
            int pausedPoition = g_value_get_int(mafw_metadata_first(metadata, MAFW_METADATA_KEY_PAUSED_POSITION));
            if (pausedPoition < 0)
                pausedPoition = 0;

            if (sourceMetadataPresent) {
                QVariant &currentValue = currentMetadata[keyName];
                if (currentValue != pausedPoition) {
                    currentValue = pausedPoition;
                    emit metadataChanged(keyName, currentValue);
                }
            } else {
                backupMetadata[keyName] = pausedPoition;
            }
        }
        // Only one piece of cover art can be shown at a given time, so both
        // source- and renderer-provided images can be stored under the same
        // key to make the life of receiving classes easier.
        else if (keyName == MAFW_METADATA_KEY_ALBUM_ART_URI) {
            keyName = MAFW_METADATA_KEY_RENDERER_ART_URI;

            // Renderer-art comes as a simple path, not a URI, contrary
            // to album-art, thus a conversion is necessary.
            const char* uri = g_value_get_string(mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_URI));
            char* filename;
            if (uri && (filename = g_filename_from_uri(uri, NULL, NULL)))
                setMetadataFromSource(keyName, QString::fromUtf8(filename));
        }
        else {
            QVariant value = toQVariant(mafw_metadata_first(metadata, (const char *) key->data));
            setMetadataFromSource(keyName, value);

            // The title will double as station name for the radion window
            if (keyName == MAFW_METADATA_KEY_TITLE)
                setMetadataFromSource(MAFW_METADATA_KEY_ORGANIZATION, value);
        }

    }

    g_list_free(keys);

    if (!sourceMetadataPresent) {
        // The video window should always receive a position to resume from to
        // work properly.
        if (backupMetadata.value(MAFW_METADATA_KEY_PAUSED_POSITION).isNull())
            backupMetadata[MAFW_METADATA_KEY_PAUSED_POSITION] = 0;

        QMap<QString,QVariant>::const_iterator currentIterator = currentMetadata.begin();
        QMap<QString,QVariant>::const_iterator backupIterator = backupMetadata.begin();

        // Compare with current metadata and emit differences
        while (true) {
            if (currentIterator == currentMetadata.end()) {
                while (backupIterator != backupMetadata.end()) {
                    // There are some additional items in backup metadata
                    emit metadataChanged(backupIterator.key(), backupIterator.value());
                    ++backupIterator;
                }
                break;
            } else if (backupIterator == backupMetadata.end()) {
                while (currentIterator != currentMetadata.end()) {
                    // There is something missing from backup metadata
                    emit metadataChanged(currentIterator.key(), QVariant());
                    ++currentIterator;
                }
                break;
            } else {
                if (currentIterator.key() > backupIterator.key()) {
                    // There are some additional items in backup metadata
                    emit metadataChanged(backupIterator.key(), backupIterator.value());
                    ++backupIterator;
                }
                else if (backupIterator.key() > currentIterator.key()) {
                    // There is something missing from backup metadata
                    emit metadataChanged(currentIterator.key(), QVariant());
                    ++currentIterator;
                }
                else {
                    if (currentIterator.value() != backupIterator.value())
                        emit metadataChanged(backupIterator.key(), backupIterator.value());
                    ++currentIterator;
                    ++backupIterator;
                }
            }
        }

        currentMetadata = backupMetadata;
        backupMetadata.clear();

        sourceMetadataPresent = true;

        emit metadataReady();
    }
}

void MetadataWatcher::onSourceMetadataChanged(QString objectId)
{
    if (objectId != currentObjectId) return;

    qDebug() << "Source metadata changed";

    mafwSource->getMetadata(currentObjectId.toUtf8(), MAFW_SOURCE_LIST(MAFW_METADATA_KEY_PAUSED_POSITION));
}

void MetadataWatcher::onRendererMetadataReceived(GHashTable *metadata, QString objectId, QString)
{
    if (objectId != currentObjectId) return;

    qDebug() << "Renderer metadata received";

    GList *keys = g_hash_table_get_keys(metadata);

    // Accept all metadata
    for (GList *key = keys; key; key = key->next)
        setMetadataFromRenderer((const char *) key->data, toQVariant(mafw_metadata_first(metadata, (const char *) key->data)));

    g_list_free(keys);
}

void MetadataWatcher::onRendererMetadataChanged(QString metadata, QVariant value)
{
    qDebug() << "Renderer metadata changed" << metadata << value;

    setMetadataFromRenderer(metadata, value);

    // Update video thumbnail
    if (metadata == MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI && currentObjectId.startsWith("localtagfs::videos")) {
        qDebug() << "Thumbnail changed" << value;
        QString thumbFile = value.toString();
        if (thumbFile.contains("mafw-gst-renderer-")) {
            QImage thumbnail(thumbFile);
            if (thumbnail.width() > thumbnail.height()) {
                // Horizontal, fill height
                thumbnail = thumbnail.scaledToHeight(124, Qt::SmoothTransformation);
                thumbnail = thumbnail.copy((thumbnail.width()-124)/2, 0, 124, 124);
            } else {
                // Vertical, fill width
                thumbnail = thumbnail.scaledToWidth(124, Qt::SmoothTransformation);
                thumbnail = thumbnail.copy(0, (thumbnail.height()-124)/2, 124, 124);
            }

            thumbFile = "/home/user/.fmp_pause_thumbnail/"
                      + QCryptographicHash::hash(currentObjectId.toUtf8(), QCryptographicHash::Md5).toHex()
                      + ".jpeg";

            thumbnail.save(thumbFile, "JPEG");

            GHashTable* metadata = mafw_metadata_new();
            mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI, qstrdup(thumbFile.toUtf8()));
            mafwTrackerSource->setMetadata(currentObjectId.toUtf8(), metadata);
            mafw_metadata_release(metadata);
        }

        // It is not necessary to inform VideosWindow directly about the change,
        // because it should receive the notification from MAFW, although that
        // can take a little bit longer.
    }
}

QVariant MetadataWatcher::toQVariant(GValue *v)
{
    switch (G_VALUE_TYPE(v)) {
        case G_TYPE_INT:
            return g_value_get_int(v);
        case G_TYPE_INT64:
            return g_value_get_int64(v);
        case G_TYPE_STRING:
            return QString::fromUtf8(g_value_get_string(v));
        case G_TYPE_BOOLEAN:
            return g_value_get_boolean(v);
        default:
            return QVariant();
    }
}
