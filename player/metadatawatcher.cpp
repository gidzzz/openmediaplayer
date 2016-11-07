#include "metadatawatcher.h"

MetadataWatcher::MetadataWatcher(MafwRegistryAdapter *mafwRegistry) :
    mafwRegistry(mafwRegistry),
    mafwRenderer(mafwRegistry->renderer()),
    mafwSource(new MafwSourceAdapter(NULL)),
    mafwTrackerSource(mafwRegistry->source(MafwRegistryAdapter::Tracker)),
    sourceMetadataPresent(false)
{
    // Initialization
    connect(mafwRenderer, SIGNAL(ready()), mafwRenderer, SLOT(getStatus()));
    connect(mafwRenderer, SIGNAL(statusReceived(MafwPlaylist*,uint,MafwPlayState,QString,QString)),
            this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState,QString)));

    // Metadata
    connect(mafwRenderer, SIGNAL(metadataChanged(QString,QVariant)),
            this, SLOT(onRendererMetadataChanged(QString,QVariant)));
    connect(mafwRenderer, SIGNAL(currentMetadataReceived(GHashTable*,QString,QString)),
            this, SLOT(onRendererMetadataReceived(GHashTable*,QString)));
    connect(mafwSource, SIGNAL(metadataResult(QString,GHashTable*,QString)),
            this, SLOT(onSourceMetadataReceived(QString,GHashTable*)));
    // Currently this is solely for the purpose of updating the paused position,
    // but it might be a good idea to keep all metadata in sync.
    connect(mafwTrackerSource, SIGNAL(metadataChanged(QString)),
            this, SLOT(onSourceMetadataChanged(QString)));
}

MafwSourceAdapter* MetadataWatcher::currentSource()
{
    return mafwSource;
}

QMap<QString,QVariant> MetadataWatcher::metadata()
{
    return currentMetadata;
}

void MetadataWatcher::setMetadataFromRenderer(QString key, QVariant value)
{
#ifdef MAFW_WORKAROUNDS
    // The renderer misreports duration of some UPnP media, so in this case give
    // priority to the source. setMetadataFromSource() implements the remaining
    // part of this workaround.
    if (key == MAFW_METADATA_KEY_DURATION
    &&  currentObjectId.startsWith("_uuid_"))
    {
        if (sourceMetadataPresent) {
            QVariant &currentValue = currentMetadata[key];
            if (currentValue.isNull()) {
                currentValue = value;
                emit metadataChanged(key, value);
            }
        } else {
            QVariant &backupValue = backupMetadata[key];
            if (backupValue.isNull())
                backupValue = value;

            QVariant &currentValue = currentMetadata[key];
            if (currentValue != value) {
                currentValue = value;
                emit metadataChanged(key, value);
            }
        }
        return;
    }
#endif

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
#ifdef MAFW_WORKAROUNDS
    // Source's part of the workaround described in setMetadataFromRenderer()
    if (key == MAFW_METADATA_KEY_DURATION
    &&  currentObjectId.startsWith("_uuid_"))
    {
        if (sourceMetadataPresent) {
            QVariant &currentValue = currentMetadata[key];
            if (currentValue != value) {
                currentValue = value;
                emit metadataChanged(key, currentValue);
            }
        } else {
            backupMetadata[key] = value;
        }
        return;
    }
#endif

    if (sourceMetadataPresent) {
        // Consider source metadata less important than renderer metadata,
        // that is do not overwrite it.
        QVariant &currentValue = currentMetadata[key];
        if (currentValue.isNull()) {
            currentValue = value;
            emit metadataChanged(key, value);
        }
    } else {
        QVariant &backupValue = backupMetadata[key];
        if (backupValue.isNull())
            backupValue = value;
    }
}

void MetadataWatcher::onStatusReceived(MafwPlaylist *, uint index, MafwPlayState, QString objectId)
{
    disconnect(mafwRenderer, SIGNAL(statusReceived(MafwPlaylist*,uint,MafwPlayState,QString,QString)),
               this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState,QString)));

    connect(mafwRenderer, SIGNAL(mediaChanged(int,QString)), this, SLOT(onMediaChanged(int,QString)));

    onMediaChanged(index, objectId);
}

void MetadataWatcher::onMediaChanged(int, QString objectId)
{
    qDebug() << "Media changed" << objectId;

    currentObjectId = objectId;

    backupMetadata.clear();
    sourceMetadataPresent = false;

    // Reset the URI as soon as possible to avoid album art misdetection
    if (currentMetadata.remove(MAFW_METADATA_KEY_URI))
        emit metadataChanged(MAFW_METADATA_KEY_URI, QVariant());

    mafwRenderer->getCurrentMetadata();

    mafwSource->bind(MAFW_SOURCE(mafwRegistry->findExtensionByUuid(objectId.left(objectId.indexOf("::")))));
    mafwSource->getMetadata(objectId, MAFW_SOURCE_ALL_KEYS);
}

void MetadataWatcher::onSourceMetadataReceived(QString objectId, GHashTable *metadata)
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
            QVariant value = MafwUtils::toQVariant(mafw_metadata_first(metadata, (const char *) key->data));
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

        // The music window should be notified about missing album art every
        // time to have a chance to look for it by its own means.
        if (!currentMetadata.contains(MAFW_METADATA_KEY_RENDERER_ART_URI))
            currentMetadata[MAFW_METADATA_KEY_RENDERER_ART_URI] = QVariant();

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

    mafwSource->getMetadata(currentObjectId, MAFW_SOURCE_LIST(MAFW_METADATA_KEY_PAUSED_POSITION));
}

void MetadataWatcher::onRendererMetadataReceived(GHashTable *metadata, QString objectId)
{
    if (objectId != currentObjectId) return;

    qDebug() << "Renderer metadata received";

    // Make sure that video codec is always emitted before audio codec so that
    // media type detection does not fail.
    if (g_hash_table_lookup(metadata, MAFW_METADATA_KEY_VIDEO_CODEC))
        setMetadataFromRenderer(MAFW_METADATA_KEY_VIDEO_CODEC, g_value_get_string(mafw_metadata_first(metadata, MAFW_METADATA_KEY_VIDEO_CODEC)));

    GList *keys = g_hash_table_get_keys(metadata);

    // Accept all metadata
    for (GList *key = keys; key; key = key->next)
        setMetadataFromRenderer((const char *) key->data, MafwUtils::toQVariant(mafw_metadata_first(metadata, (const char *) key->data)));

    g_list_free(keys);
}

void MetadataWatcher::onRendererMetadataChanged(QString metadata, QVariant value)
{
    qDebug() << "Renderer metadata changed" << metadata << value;

    if (metadata == MAFW_METADATA_KEY_AUDIO_CODEC) {
        // If video codec info is available, then audio codec info should not
        // arrive before it for media type detection to work. Make an additional
        // metadata request instead of handling it immediately, in case the
        // renderer has the desired information, but has not emitted it yet.
        mafwRenderer->getCurrentMetadata();
    } else {
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
                mafwTrackerSource->setMetadata(currentObjectId, metadata);
                mafw_metadata_release(metadata);
            }

            // It is not necessary to inform VideosWindow directly about the change,
            // because it should receive the notification from MAFW, although that
            // can take a little bit longer.
        }
    }
}
