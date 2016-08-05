#ifndef MAFWRENDERERADAPTER_H
#define MAFWRENDERERADAPTER_H

#include <QObject>

#include <QVariant>
#include <QDebug>

#include <libmafw/mafw.h>
#include <libmafw-shared/mafw-shared.h>

#ifdef MAFW_WORKAROUNDS
    class MafwRendererAdapter;
    #include "mafwplaylistadapter.h"
#endif

class MafwRendererAdapter : public QObject
{
    Q_OBJECT

public:
    MafwRendererAdapter(const QString &uuid);
    ~MafwRendererAdapter();

    bool isReady();

public slots:
    // Exposed operations
    void play();
    void playObject(const QString &objectId);
    void playUri(const QString &uri);
    void stop();
    void pause();
    void resume();
    void getStatus();
    bool assignPlaylist(MafwPlaylist *playlist);
    void next();
    void previous();
    void gotoIndex(uint index);
    void setPosition(MafwRendererSeekMode mode, int seconds);
    void getPosition();
    void getCurrentMetadata();

    // Exposed properties
    void setVolume(int volume);
    void getVolume();
    void setXid(uint Xid);
    void setErrorPolicy(uint errorPolicy);
    void setColorKey(int colorKey);

signals:
    void ready();

    // Exposed signals
    void bufferingInfo(float status);
    void mediaChanged(int index, const QString &objectId);
    void metadataChanged(const QString &metadata, const QVariant &value);
    void playlistChanged(GObject *playlist);
    void stateChanged(MafwPlayState state);
    void propertyChanged(const QString &name, const QVariant &value);

    // Exposed operation callbacks
    void playExecuted(const QString &error);
    void playObjectExecuted(const QString &error);
    void playUriExecuted(const QString &error);
    void stopExecuted(const QString &error);
    void pauseExecuted(const QString &error);
    void resumeExecuted(const QString &error);
    void statusReceived(MafwPlaylist *playlist, uint index, MafwPlayState state, const QString &objectId, const QString &error);
    void nextExecuted(const QString &error);
    void previousExecuted(const QString &error);
    void gotoIndexExecuted(const QString &error);
    void positionReceived(int position, const QString &error);
    void currentMetadataReceived(GHashTable *metadata, const QString &objectId, const QString &error);

    // Exposed property callbacks
    void volumeReceived(int volume, const QString &error);

private:
    MafwRenderer *renderer;
    QString m_uuid;

    void bind(MafwRenderer *renderer);

    // Signal handlers
    static void onBufferingInfo(MafwRenderer *, gfloat status, gpointer self);
    static void onMediaChanged(MafwRenderer *, gint index, gchar *objectId, gpointer self);
    static void onMetadataChanged(MafwRenderer *, gchar *name, GValueArray *value, gpointer self);
    static void onPlaylistChanged(MafwRenderer *, GObject *playlist, gpointer self);
    static void onStateChanged(MafwRenderer *, gint state, gpointer self);
    static void onPropertyChanged(MafwExtension *, gchar *name, GValue *value, gpointer self);

    // Operation callbacks
    static void onPlayExecuted(MafwRenderer *, gpointer self, const GError *error);
    static void onPlayObjectExecuted(MafwRenderer *, gpointer self, const GError *error);
    static void onPlayUriExecuted(MafwRenderer *, gpointer self, const GError *error);
    static void onStopExecuted(MafwRenderer *, gpointer self, const GError *error);
    static void onPauseExecuted(MafwRenderer *, gpointer self, const GError *error);
    static void onResumeExecuted(MafwRenderer *, gpointer self, const GError *error);
    static void onStatusReceived(MafwRenderer *, MafwPlaylist *playlist, guint index, MafwPlayState state, const gchar *objectId, gpointer self, const GError *error);
    static void onNextExecuted(MafwRenderer *, gpointer self, const GError *error);
    static void onPreviousExecuted(MafwRenderer *, gpointer self, const GError *error);
    static void onGotoIndexExecuted(MafwRenderer *, gpointer self, const GError *error);
    static void onPositionReceived(MafwRenderer *, gint position, gpointer self, const GError *error);
    static void onCurrentMetadataReceived(MafwRenderer *, const gchar *objectId, GHashTable *metadata, gpointer self, const GError *error);

    // Property callbacks
    static void onVolumeReceived(MafwExtension *, const gchar *, GValue *value, gpointer self, const GError *error);

#ifdef MAFW_WORKAROUNDS
    MafwPlaylistAdapter *playlist;
    friend class MafwRegistryAdapter;
#endif

private slots:
    void onRendererAdded(MafwRenderer *renderer);
    void onRendererRemoved(MafwRenderer *renderer);
};

#endif // MAFWRENDERERADAPTER_H
