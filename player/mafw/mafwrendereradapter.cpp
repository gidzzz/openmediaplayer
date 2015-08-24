#include "mafwrendereradapter.h"

#include "mafwregistryadapter.h"
#include "mafwutils.h"

// NOTE: Unlike with MafwSourceAdapter, instance tracking is not present, thus
// objects of this class are not disposable.

MafwRendererAdapter::MafwRendererAdapter(const QString &uuid) :
    renderer(NULL),
    m_uuid(uuid)
{
    connect(MafwRegistryAdapter::get(), SIGNAL(rendererAdded(MafwRenderer*)), this, SLOT(onRendererAdded(MafwRenderer*)));

    bind(MAFW_RENDERER(MafwRegistryAdapter::get()->findExtensionByUuid(uuid)));
}

MafwRendererAdapter::~MafwRendererAdapter()
{
    bind(NULL);
}

bool MafwRendererAdapter::isReady()
{
    return renderer;
}

void MafwRendererAdapter::bind(MafwRenderer *renderer)
{
    // Check if there is anything to do
    if (renderer == this->renderer)
        return;

    if (renderer) {
        // Unbind current renderer, if set, before proceeding
        if (this->renderer)
            bind(NULL);

        // Bind
        g_object_ref(renderer);
        g_signal_connect(renderer, "buffering-info"  , G_CALLBACK(&onBufferingInfo)  , static_cast<void*>(this));
        g_signal_connect(renderer, "media-changed"   , G_CALLBACK(&onMediaChanged)   , static_cast<void*>(this));
        g_signal_connect(renderer, "metadata-changed", G_CALLBACK(&onMetadataChanged), static_cast<void*>(this));
        g_signal_connect(renderer, "playlist-changed", G_CALLBACK(&onPlaylistChanged), static_cast<void*>(this));
        g_signal_connect(renderer, "state-changed"   , G_CALLBACK(&onStateChanged)   , static_cast<void*>(this));

        this->renderer = renderer;
        m_uuid = mafw_extension_get_uuid(MAFW_EXTENSION(renderer));

        // Watch only for removals
        disconnect(MafwRegistryAdapter::get(), SIGNAL(rendererAdded(MafwRenderer*)), this, SLOT(onRendererAdded(MafwRenderer*)));
        connect(MafwRegistryAdapter::get(), SIGNAL(rendererRemoved(MafwRenderer*)), this, SLOT(onRendererRemoved(MafwRenderer*)));

        emit ready();
    } else {
        // Unbind
        g_signal_handlers_disconnect_matched(this->renderer, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, this);
        g_object_unref(this->renderer);

        this->renderer = NULL;

        // Watch only for additions
        disconnect(MafwRegistryAdapter::get(), SIGNAL(rendererRemoved(MafwRenderer*)), this, SLOT(onRendererRemoved(MafwRenderer*)));
        connect(MafwRegistryAdapter::get(), SIGNAL(rendererAdded(MafwRenderer*)), this, SLOT(onRendererAdded(MafwRenderer*)));
    }
}

void MafwRendererAdapter::onRendererAdded(MafwRenderer *renderer)
{
    if (m_uuid == mafw_extension_get_uuid(MAFW_EXTENSION(renderer)))
        bind(renderer);
}

void MafwRendererAdapter::onRendererRemoved(MafwRenderer *renderer)
{
    if (m_uuid == mafw_extension_get_uuid(MAFW_EXTENSION(renderer)))
        bind(NULL);
}

//--- Exposed operations -------------------------------------------------------

void MafwRendererAdapter::play()
{
    if (!renderer) return;

#ifdef MAFW_WORKAROUNDS
    // Early play() or gotoIndex() seems reliable only for smaller libraries.
    // For bigger ones something probably doesn't have enough time to ready up.
    // Possible workaround is to call size() first, so when it returns, we know
    // that play() can be successfully called. That's only a theory, but it
    // seems to work.
    playlist->size();
#endif

    mafw_renderer_play(renderer, onPlayExecuted, this);
}

void MafwRendererAdapter::playObject(const QString &objectId)
{
    if (renderer)
        mafw_renderer_play_object(renderer, objectId.toUtf8(), onPlayObjectExecuted, this);
}

void MafwRendererAdapter::playUri(const QString &uri)
{
    if (renderer)
        mafw_renderer_play_uri(renderer, uri.toUtf8(), onPlayUriExecuted, this);
}

void MafwRendererAdapter::stop()
{
    if (renderer)
        mafw_renderer_stop(renderer, onStopExecuted, this);
}

void MafwRendererAdapter::pause()
{
    if (renderer)
        mafw_renderer_pause(renderer, onPauseExecuted, this);
}

void MafwRendererAdapter::resume()
{
    if (renderer)
        mafw_renderer_resume(renderer, onResumeExecuted, this);
}

void MafwRendererAdapter::getStatus()
{
    if (renderer)
        mafw_renderer_get_status(renderer, onStatusReceived, this);
}

bool MafwRendererAdapter::assignPlaylist(MafwPlaylist *playlist)
{
    return renderer && mafw_renderer_assign_playlist(renderer, playlist, NULL);
}

void MafwRendererAdapter::next()
{
    if (renderer)
        mafw_renderer_next(renderer, onNextExecuted, this);
}

void MafwRendererAdapter::previous()
{
    if (renderer)
        mafw_renderer_previous(renderer, onPreviousExecuted, this);
}

void MafwRendererAdapter::gotoIndex(uint index)
{
    if (!renderer) return;

#ifdef MAFW_WORKAROUNDS
    // Explained in play()
    playlist->size();
#endif

    mafw_renderer_goto_index(renderer, index, onGotoIndexExecuted, this);
}

void MafwRendererAdapter::setPosition(MafwRendererSeekMode mode, int seconds)
{
    if (renderer)
        mafw_renderer_set_position(renderer, mode, seconds, onPositionReceived, this);
}

void MafwRendererAdapter::getPosition()
{
    if (renderer)
        mafw_renderer_get_position(renderer, onPositionReceived, this);
}

void MafwRendererAdapter::getCurrentMetadata()
{
    if (renderer)
        mafw_renderer_get_current_metadata(renderer, onCurrentMetadataReceived, this);
}

//--- Exposed properties -------------------------------------------------------

void MafwRendererAdapter::setVolume(int volume)
{
    // This uint property is masked as int for compatibility with Qt
    if (renderer)
        mafw_extension_set_property_uint(MAFW_EXTENSION(renderer), MAFW_PROPERTY_RENDERER_VOLUME, volume);
}

void MafwRendererAdapter::getVolume()
{
    if (renderer)
        mafw_extension_get_property(MAFW_EXTENSION(renderer), MAFW_PROPERTY_RENDERER_VOLUME, onVolumeReceived, this);
}

void MafwRendererAdapter::setXid(uint xid)
{
    if (renderer)
        mafw_extension_set_property_uint(MAFW_EXTENSION(renderer), MAFW_PROPERTY_RENDERER_XID, xid);
}

void MafwRendererAdapter::setErrorPolicy(uint errorPolicy)
{
    if (renderer)
        mafw_extension_set_property_uint(MAFW_EXTENSION(renderer), MAFW_PROPERTY_RENDERER_ERROR_POLICY, errorPolicy);
}

void MafwRendererAdapter::setColorKey(int colorKey)
{
    // Despite what the MAFW documentation says, this a writable property
    if (renderer)
        mafw_extension_set_property_int(MAFW_EXTENSION(this->renderer), MAFW_PROPERTY_RENDERER_COLORKEY, colorKey);
}

//--- Signal handlers ----------------------------------------------------------

void MafwRendererAdapter::onBufferingInfo(MafwRenderer *, gfloat status, gpointer self)
{
    emit static_cast<MafwRendererAdapter*>(self)->bufferingInfo(status);
}

void MafwRendererAdapter::onMediaChanged(MafwRenderer *, gint index, gchar *objectId, gpointer self)
{
    emit static_cast<MafwRendererAdapter*>(self)->mediaChanged(index, QString::fromUtf8(objectId));
}

void MafwRendererAdapter::onMetadataChanged(MafwRenderer*, gchar *name, GValueArray *value, gpointer self)
{
    if (value->n_values == 1) {
        GValue *v = g_value_array_get_nth(value, 0);

        switch(G_VALUE_TYPE(v)) {
            case G_TYPE_BOOLEAN:
                emit static_cast<MafwRendererAdapter*>(self)->metadataChanged(name, g_value_get_boolean(v)); break;
            case G_TYPE_INT:
                emit static_cast<MafwRendererAdapter*>(self)->metadataChanged(name, g_value_get_int(v)); break;
            case G_TYPE_INT64:
                emit static_cast<MafwRendererAdapter*>(self)->metadataChanged(name, g_value_get_int64(v)); break;
            case G_TYPE_STRING:
                emit static_cast<MafwRendererAdapter*>(self)->metadataChanged(name, QString::fromUtf8(g_value_get_string(v))); break;
            default:
                qDebug() << "Unsupported metadata type" << G_VALUE_TYPE_NAME(v) << "for" << name; break;
        }
    } else {
        qDebug() << "Unsupported metadata count" << value->n_values << "for" << name;
    }
}

void MafwRendererAdapter::onPlaylistChanged(MafwRenderer *, GObject *playlist, gpointer self)
{
    emit static_cast<MafwRendererAdapter*>(self)->playlistChanged(playlist);
}

void MafwRendererAdapter::onStateChanged(MafwRenderer *, gint state, gpointer self)
{
    emit static_cast<MafwRendererAdapter*>(self)->stateChanged(static_cast<MafwPlayState>(state));
}

//--- Callbacks ----------------------------------------------------------------

void MafwRendererAdapter::onPlayExecuted(MafwRenderer *, gpointer self, const GError *error)
{
    emit static_cast<MafwRendererAdapter*>(self)->playExecuted(MafwUtils::toQString(error));

#ifdef MAFW_WORKAROUNDS
    // MAFW behaves inconsistently when it comes to assigning items to
    // a renderer. It can end up without any media to play despite the playlist
    // not being empty. gotoIndex() can fix the issue, but next() is probably
    // better due to being shuffle-friendly.
    static int retries = 0;
    if (error && error->code == MAFW_RENDERER_ERROR_NO_MEDIA) {
        if (static_cast<MafwRendererAdapter*>(self)->playlist->size()) {
            if (retries < 5) {
                qDebug() << "Trying to recover from MAFW_RENDERER_ERROR_NO_MEDIA";
                ++retries;
                static_cast<MafwRendererAdapter*>(self)->next();
                static_cast<MafwRendererAdapter*>(self)->play();
            } else {
                qDebug() << "Giving up on MAFW_RENDERER_ERROR_NO_MEDIA";
            }
            return;
        }
    }
    retries = 0;
#endif
}

void MafwRendererAdapter::onPlayObjectExecuted(MafwRenderer *, gpointer self, const GError *error)
{
    emit static_cast<MafwRendererAdapter*>(self)->playObjectExecuted(MafwUtils::toQString(error));
}

void MafwRendererAdapter::onPlayUriExecuted(MafwRenderer *, gpointer self, const GError *error)
{
    emit static_cast<MafwRendererAdapter*>(self)->playUriExecuted(MafwUtils::toQString(error));
}

void MafwRendererAdapter::onStopExecuted(MafwRenderer *, gpointer self, const GError *error)
{
    emit static_cast<MafwRendererAdapter*>(self)->stopExecuted(MafwUtils::toQString(error));
}

void MafwRendererAdapter::onPauseExecuted(MafwRenderer *, gpointer self, const GError *error)
{
    emit static_cast<MafwRendererAdapter*>(self)->pauseExecuted(MafwUtils::toQString(error));
}

void MafwRendererAdapter::onResumeExecuted(MafwRenderer *, gpointer self, const GError *error)
{
    emit static_cast<MafwRendererAdapter*>(self)->resumeExecuted(MafwUtils::toQString(error));
}

void MafwRendererAdapter::onStatusReceived(MafwRenderer *, MafwPlaylist *playlist, guint index, MafwPlayState state, const gchar *objectId, gpointer self, const GError *error)
{
    emit static_cast<MafwRendererAdapter*>(self)->statusReceived(playlist, index, state, QString::fromUtf8(objectId), MafwUtils::toQString(error));
}

void MafwRendererAdapter::onNextExecuted(MafwRenderer *, gpointer self, const GError *error)
{
    emit static_cast<MafwRendererAdapter*>(self)->nextExecuted(MafwUtils::toQString(error));
}

void MafwRendererAdapter::onPreviousExecuted(MafwRenderer *, gpointer self, const GError *error)
{
    emit static_cast<MafwRendererAdapter*>(self)->previousExecuted(MafwUtils::toQString(error));
}

void MafwRendererAdapter::onGotoIndexExecuted(MafwRenderer *, gpointer self, const GError *error)
{
    emit static_cast<MafwRendererAdapter*>(self)->gotoIndexExecuted(MafwUtils::toQString(error));
}

void MafwRendererAdapter::onPositionReceived(MafwRenderer *, gint position, gpointer self, const GError *error)
{
    emit static_cast<MafwRendererAdapter*>(self)->positionReceived(position, MafwUtils::toQString(error));
}

void MafwRendererAdapter::onCurrentMetadataReceived(MafwRenderer *, const gchar *objectId, GHashTable *metadata, gpointer self, const GError *error)
{
    emit static_cast<MafwRendererAdapter*>(self)->currentMetadataReceived(metadata, QString::fromUtf8(objectId), MafwUtils::toQString(error));
}

void MafwRendererAdapter::onVolumeReceived(MafwExtension *, const gchar *, GValue *value, gpointer self, const GError *error)
{
    emit static_cast<MafwRendererAdapter*>(self)->volumeReceived(g_value_get_uint(value), MafwUtils::toQString(error));
}
