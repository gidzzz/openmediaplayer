#include "missioncontrol.h"

MissionControl* MissionControl::instance = NULL;

MissionControl* MissionControl::acquire()
{
    return instance ? instance : instance = new MissionControl();
}

MissionControl::MissionControl() :
    m_sleeper(NULL)
{
}

void MissionControl::setFactory(MafwAdapterFactory *factory)
{
    mafwRenderer = factory->getRenderer();
    mafwTrackerSource = factory->getTrackerSource();

    // Beware, it is possible to miss rendererReady() if not connected right
    // after creating the renderer.
    connect(mafwRenderer, SIGNAL(rendererReady()), mafwRenderer, SLOT(getStatus()));
    connect(mafwRenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));
    connect(mafwRenderer, SIGNAL(mediaChanged(int,char*)), this, SLOT(onMediaChanged(int,char*)));
    connect(mafwRenderer, SIGNAL(metadataChanged(QString,QVariant)), this, SLOT(onMetadataChanged(QString,QVariant)));
}

Sleeper* MissionControl::sleeper()
{
    if (!m_sleeper) {
        m_sleeper = new Sleeper(this, mafwRenderer);
        connect(m_sleeper, SIGNAL(finished()), this, SLOT(onSleeperTimeout()));
    }

    return m_sleeper;
}

void MissionControl::onSleeperTimeout()
{
    m_sleeper->deleteLater();
    m_sleeper = NULL;
}

void MissionControl::onStatusReceived(MafwPlaylist *, uint, MafwPlayState, const char *objectId, QString)
{
    disconnect(mafwRenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
               this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));

    currentObjectId = QString::fromUtf8(objectId);
}

void MissionControl::onMediaChanged(int, char *objectId)
{
    currentObjectId = QString::fromUtf8(objectId);
}

void MissionControl::onMetadataChanged(QString metadata, QVariant value)
{
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
        // because it should receive the notification from MAFW, although that can
        // take a little bit longer.
    }
}
