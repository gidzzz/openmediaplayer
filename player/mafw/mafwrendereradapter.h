#ifndef _MAFW_RENDERER_ADAPTER_HPP_
#define _MAFW_RENDERER_ADAPTER_HPP_

#include <QObject>
#include <QVariant>
#include <QString>
#include <QDebug>

#include <libmafw/mafw.h>
#include <libmafw-shared/mafw-shared.h>
#include <libplayback/playback.h>
#include <glib.h>

#include "mafwrenderersignalhelper.h"

#ifdef MAFW_WORKAROUNDS
    class MafwRendererAdapter;
    class MafwPlaylistAdapter;
    #include "mafwplaylistadapter.h"
#endif

#define MEDIAPLAYER_RENDERER "Mafw-Gst-Renderer"
#define MEDIAPLAYER_SOURCE "Mafw-Tracker-Source"

class MafwRendererAdapter : public QObject
{
  Q_OBJECT

    friend class MafwRendererSignalHelper;
 public:
  MafwRendererAdapter();
  ~MafwRendererAdapter() { }

  static void onRendererAdded(MafwRegistry* mafw_registry, GObject* renderer, gpointer user_data);
  static void onRendererRemoved(MafwRegistry* mafw_registry, GObject* renderer,gpointer user_data);

  static void onBufferingInfo(MafwRenderer* mafw_renderer, gfloat state, gpointer user_data);
  static void onMediaChanged(MafwRenderer* mafw_renderer, gint index, gchar* object_id, gpointer user_data);
  static void onMetadataChanged(MafwRenderer* mafw_renderer, gchar* name, GValueArray* value, gpointer user_data);
  static void onPlaylistChanged(MafwRenderer* mafw_renderer, GObject* playlist, gpointer user_data);
  static void onStateChanged(MafwRenderer* mafw_renderer, gint state, gpointer user_data);

  static void playback_state_req_handler(pb_playback_t *pb, pb_state_e req_state, pb_req_t *ext_req, void *data);
  static void playback_state_req_callback(pb_playback_t *pb, pb_state_e granted_state, const char *reason, pb_req_t *req, void *data);

  void enablePlayback(bool enable, bool compatible = false);
  bool isRendererReady();

 public slots:
  void play();
  void playObject(const char* object_id);
  void playURI(const char* uri);
  void stop();
  void pause();
  void resume();
  void getStatus();
  void next();
  void previous();
  void gotoIndex(uint index);
  void setPosition(MafwRendererSeekMode mode, int seconds);
  void getPosition();
  void getCurrentMetadata();
  void setVolume(int volume);
  void getVolume();
  void setWindowXid(uint Xid);
  void setColorKey(int colorKey);
  void setErrorPolicy(uint errorPolicy);

  bool assignPlaylist(MafwPlaylist* playlist);

 signals:
  // MafwRenderer signals
  void rendererReady();
  void bufferingInfo(float status);
  void mediaChanged(int index, char* objectId);
  void metadataChanged(QString metadata, QVariant value);
  void playlistChanged(GObject* playlist);
  void stateChanged(int newState);

  // Mafw callbacks as signals
  void signalPlay(QString error);
  void signalPlayURI(QString error);
  void signalPlayObject(QString error);
  void signalStop(QString error);
  void signalPause(QString error);
  void signalResume(QString error);
  void signalGetStatus(MafwPlaylist* playlist, uint index, MafwPlayState state, const char* object_id, QString error);
  void signalNext(QString error);
  void signalPrevious(QString error);
  void signalGotoIndex(QString error);
  void signalSetPosition(int position, QString error);
  void signalGetPosition(int position, QString error);
  void signalGetCurrentMetadata(GHashTable *metadata, QString object_id, QString error);
  void signalGetVolume(int volume);

 private slots:
  void initializePlayback(MafwPlaylist*, uint, MafwPlayState state, const char*, QString);

 private:
  enum Action
  {
    Play,
    Pause,
    Resume,
    Stop,
    Dummy
  };

  struct req_state_cb_payload
  {
    MafwRendererAdapter* adapter;
    Action action;
  };

  void findRenderer();
  void connectRegistrySignals();
  void connectRendererSignals();
  void disconnectRendererSignals();
  MafwRegistry* mafw_registry;
  MafwRenderer* mafw_renderer;
  pb_playback_t* playback;
  bool compatiblePlayback;
  GValue GVolume;

#ifdef MAFW_WORKAROUNDS
  MafwPlaylistAdapter* playlist;
  friend class MafwRegistryAdapter;
#endif
};
#endif
