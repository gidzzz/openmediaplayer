#include "metadatawatcher.h"

MetadataWatcher::MetadataWatcher(MafwAdapterFactory *factory) :
    mafwRenderer(factory->getRenderer()),
    mafwSource(factory->getTempSource()),
    mafwTrackerSource(factory->getTrackerSource())
{
    // Initialization
    connect(mafwRenderer, SIGNAL(rendererReady()), mafwRenderer, SLOT(getStatus()));
    connect(mafwRenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));

    connect(mafwRenderer, SIGNAL(mediaChanged(int,char*)), this, SLOT(onMediaChanged(int,char*)));

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

    initMetadata();
}

QMap<QString,QVariant> MetadataWatcher::metadata()
{
    return m_metadata;
}

void MetadataWatcher::initMetadata()
{
    setMetadata(MAFW_METADATA_KEY_TITLE,  QVariant());
    setMetadata(MAFW_METADATA_KEY_ARTIST, QVariant());
    setMetadata(MAFW_METADATA_KEY_ALBUM,  QVariant());

    setMetadata(MAFW_METADATA_KEY_DURATION, QVariant());
    setMetadata(MAFW_METADATA_KEY_IS_SEEKABLE, QVariant());

    setMetadata(MAFW_METADATA_KEY_URI,  QVariant());
    setMetadata(MAFW_METADATA_KEY_MIME, QVariant());

    setMetadata(MAFW_METADATA_KEY_RENDERER_ART_URI, QVariant());

    setMetadata(MAFW_METADATA_KEY_PAUSED_POSITION, QVariant());
}

void MetadataWatcher::setMetadata(QString key, QVariant value)
{
    m_metadata[key] = value;

    emit metadataChanged(key, value);
}

void MetadataWatcher::extractMetadata(const char *key, GHashTable *metadata)
{
    GValue *v = mafw_metadata_first(metadata, key);
    switch (G_VALUE_TYPE(v)) {
        case G_TYPE_INT:
            setMetadata(key, g_value_get_int(v));
            break;
        case G_TYPE_INT64:
            setMetadata(key, g_value_get_int64(v));
            break;
        case G_TYPE_STRING:
            setMetadata(key, QString::fromUtf8(g_value_get_string(v)));
            break;
        case G_TYPE_BOOLEAN:
            setMetadata(key, g_value_get_boolean(v));
            break;
        default:
            break;
    }
}

void MetadataWatcher::onStatusReceived(MafwPlaylist *, uint index, MafwPlayState, const char *objectId, QString)
{
    disconnect(mafwRenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
               this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));

    onMediaChanged(index, (char *) objectId);
}

void MetadataWatcher::onMediaChanged(int, char *objectId)
{
    qDebug() << "Media changed" << objectId;

    currentObjectId = QString::fromUtf8(objectId);

    m_metadata.clear();
    initMetadata();

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

        // Only one piece of cover art can be shown at a given time, so both
        // source- and renderer-provided images can be stored under the same key
        // to make the life of receiving classes easier,
        if (keyName == MAFW_METADATA_KEY_ALBUM_ART_URI) {
            keyName = MAFW_METADATA_KEY_RENDERER_ART_URI;
            if (m_metadata.value(keyName).isNull()) {
                // Renderer-art comes as a simple path, not a URI, contrary to
                // album-art, thus a conversion is necessary.
                const char* uri = g_value_get_string(mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_URI));
                char* filename;
                if (uri && (filename = g_filename_from_uri(uri, NULL, NULL)))
                    setMetadata(MAFW_METADATA_KEY_RENDERER_ART_URI, QString::fromUtf8(filename));
            }
        }
        // Paused position might require some processing to prevent confusion in
        // the video window.
        else if (keyName == MAFW_METADATA_KEY_PAUSED_POSITION) {
            int pausedPoition = g_value_get_int(mafw_metadata_first(metadata, MAFW_METADATA_KEY_PAUSED_POSITION));
            setMetadata(MAFW_METADATA_KEY_PAUSED_POSITION, pausedPoition < 0 ? 0 : pausedPoition);
        }
        // Consider source metadata less important than renderer metadata, that
        // is do not overwrite it.
        else if (m_metadata.value(keyName).isNull()) {
            extractMetadata((const char *) key->data, metadata);

            if (keyName == MAFW_METADATA_KEY_TITLE) {
                // The title will double as station name for the radion window
                setMetadata(MAFW_METADATA_KEY_ORGANIZATION, m_metadata[MAFW_METADATA_KEY_TITLE]);
            }
        }
    }

    g_list_free(keys);

    // The video window should always receive a position to resume from to work
    // properly.
    if (m_metadata.value(MAFW_METADATA_KEY_PAUSED_POSITION).isNull())
        setMetadata(MAFW_METADATA_KEY_PAUSED_POSITION, 0);
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
        extractMetadata((const char *) key->data, metadata);

    g_list_free(keys);
}

void MetadataWatcher::onRendererMetadataChanged(QString metadata, QVariant value)
{
    qDebug() << "Renderer metadata changed" << metadata << value;

    setMetadata(metadata, value);

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
