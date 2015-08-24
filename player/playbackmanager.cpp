#include "playbackmanager.h"

PlaybackManager::PlaybackManager(MafwRendererAdapter *mafwRenderer) :
    mafwRenderer(mafwRenderer),
    playback(NULL)
{
}

void PlaybackManager::enable(bool enable, bool compatible)
{
    if (enable) {
        if (playback) {
            if (this->compatible != compatible) {
                this->enable(false);
                this->enable(true, compatible);
            }
        } else {
            this->compatible = compatible;

            connect(mafwRenderer, SIGNAL(statusReceived(MafwPlaylist*,uint,MafwPlayState,QString,QString)),
                    this, SLOT(initialize(MafwPlaylist*,uint,MafwPlayState)));

            mafwRenderer->getStatus();
        }
    } else {
        if (playback) {
            disconnect(mafwRenderer, SIGNAL(stateChanged(MafwPlayState)), this, SLOT(onStateChanged(MafwPlayState)));
            disconnect(mafwRenderer, SIGNAL(playExecuted(QString)), this, SLOT(onPlayExecuted(QString)));
            disconnect(mafwRenderer, SIGNAL(playObjectExecuted(QString)), this, SLOT(onPlayExecuted(QString)));
            disconnect(mafwRenderer, SIGNAL(playUriExecuted(QString)), this, SLOT(onPlayExecuted(QString)));
            disconnect(mafwRenderer, SIGNAL(resumeExecuted(QString)), this, SLOT(onPlayExecuted(QString)));
            disconnect(mafwRenderer, SIGNAL(statusReceived(MafwPlaylist*,uint,MafwPlayState,QString,QString)),
                       this, SLOT(initialize(MafwPlaylist*,uint,MafwPlayState)));

            pb_playback_destroy(playback);
            playback = NULL;
        }
    }
}

void PlaybackManager::request(pb_state_e state)
{
    pb_playback_req_state(playback, state, requestCallback, NULL);
}

void PlaybackManager::requestHandler(pb_playback_t *, pb_state_e, pb_req_t *, void *)
{
    // Perhaps this could be used to handle incoming calls.
    // Currently that is accomplished using MCE in MissionControl.
}

// Waiting for an opinion from libplayback can take so long that there can be
// a significant delay between disconnecting a headset and pausing music and
// play/pause button also seems a bit laggy. Because of that it might be a good
// idea to not wait for permission, but rather do what we have to do and later
// only tell the playback manager what we wanted.
void PlaybackManager::requestCallback(pb_playback_t *pb, pb_state_e granted_state, const char *reason, pb_req_t *req, void *)
{
    if (granted_state == PB_STATE_NONE)
        qDebug() << "Playback state request denied:" << reason;

    pb_playback_req_completed(pb, req);
}

void PlaybackManager::initialize(MafwPlaylist *, uint, MafwPlayState state)
{
    disconnect(mafwRenderer, SIGNAL(statusReceived(MafwPlaylist*,uint,MafwPlayState,QString,QString)),
               this, SLOT(initialize(MafwPlaylist*,uint,MafwPlayState)));

    playback = pb_playback_new(dbus_g_connection_get_connection(dbus_g_bus_get(DBUS_BUS_SESSION, NULL)),
                               PB_CLASS_MEDIA, state == Playing && !compatible ? PB_STATE_PLAY : PB_STATE_STOP,
                               requestHandler, NULL);

    connect(mafwRenderer, SIGNAL(stateChanged(MafwPlayState)), this, SLOT(onStateChanged(MafwPlayState)));
    if (compatible) {
        connect(mafwRenderer, SIGNAL(playExecuted(QString)), this, SLOT(onPlayExecuted(QString)));
        connect(mafwRenderer, SIGNAL(playObjectExecuted(QString)), this, SLOT(onPlayExecuted(QString)));
        connect(mafwRenderer, SIGNAL(playUriExecuted(QString)), this, SLOT(onPlayExecuted(QString)));
        connect(mafwRenderer, SIGNAL(resumeExecuted(QString)), this, SLOT(onPlayExecuted(QString)));
    }
}

void PlaybackManager::onStateChanged(MafwPlayState state)
{
    if (state == Paused || state == Stopped) {
        request(PB_STATE_STOP);
    } else if (state == Playing && !compatible) {
        request(PB_STATE_PLAY);
    }
}

void PlaybackManager::onPlayExecuted(QString error)
{
    if (error.isNull())
        request(PB_STATE_PLAY);
}
