#ifndef PLAYBACKMANAGER_H
#define PLAYBACKMANAGER_H

#include <QObject>

#include <libplayback/playback.h>

#include "mafw/mafwrendereradapter.h"

class PlaybackManager : public QObject
{
    Q_OBJECT

public:
    PlaybackManager(MafwRendererAdapter *mafwRenderer);

    void enable(bool enable, bool compatible = false);

private:
    MafwRendererAdapter *mafwRenderer;
    pb_playback_t *playback;
    bool compatible;

    void request(pb_state_e state);

    static void requestHandler(pb_playback_t *, pb_state_e, pb_req_t *, void *);
    static void requestCallback(pb_playback_t *pb, pb_state_e granted_state, const char *reason, pb_req_t *req, void *);

private slots:
    void initialize(MafwPlaylist *, uint, MafwPlayState state);
    void onStateChanged(MafwPlayState state);
    void onPlayExecuted(QString error);
};

#endif // PLAYBACKMANAGER_H
