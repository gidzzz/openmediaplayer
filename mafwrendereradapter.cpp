#include "mafwrendereradapter.h"

MafwRendererAdapter::MafwRendererAdapter()
{
  g_warning("start\n");
  mafw_registry = MAFW_REGISTRY(mafw_registry_get_instance());
  mafw_shared_init(mafw_registry, NULL);
  findRenderer();
  connectRegistrySignals();

}

void MafwRendererAdapter::findRenderer()
{
  if(mafw_registry)
  {
    GList* renderer_list = mafw_registry_get_renderers(mafw_registry);
    if(renderer_list)
    {
      GList* renderer_elem = renderer_list;
      while(renderer_elem)
      {
	MafwRenderer* mafw_renderer = MAFW_RENDERER(renderer_elem->data);
	g_warning("renderer: %s\n", mafw_extension_get_name(MAFW_EXTENSION(mafw_renderer)));
	if(g_strcmp0(mafw_extension_get_name(MAFW_EXTENSION(mafw_renderer)),MEDIAPLAYER_RENDERER) == 0)
	{
	  g_object_ref(mafw_renderer);
	  this->mafw_renderer = mafw_renderer;
	  connectRendererSignals();
	}
      }
    }  
    else
    { 
      g_warning("no renderer\n");
    }
  }
  else
  {
    g_warning("no rgistry\n");
  }
}

void MafwRendererAdapter::connectRegistrySignals()
{
  g_signal_connect(mafw_registry, 
		   "renderer_added", 
		   G_CALLBACK(&onRendererAdded), 
		   static_cast<void*>(this));
  
  g_signal_connect(mafw_registry, 
		   "renderer_removed", 
		   G_CALLBACK(&onRendererRemoved), 
		   static_cast<void*>(this));
}

void MafwRendererAdapter::connectRendererSignals()
{
  g_print("connect renderer signals\n");
  g_signal_connect(mafw_renderer,
		   "buffering-info",
		   G_CALLBACK(&onBufferingInfo),
		   static_cast<void*>(this));
  g_signal_connect(mafw_renderer,
		   "media-changed",
		   G_CALLBACK(&onMediaChanged),
		   static_cast<void*>(this));
  g_signal_connect(mafw_renderer,
		   "metadata-changed",
		   G_CALLBACK(&onMetadataChanged),
		   static_cast<void*>(this));
  g_signal_connect(mafw_renderer,
		   "playlist-changed",
		   G_CALLBACK(&onPlaylistChanged),
		   static_cast<void*>(this));
  g_signal_connect(mafw_renderer,
		   "state-changed",
		   G_CALLBACK(&onStateChanged),
		   static_cast<void*>(this));
}

void MafwRendererAdapter::disconnectRendererSignals()
{
  
}

void MafwRendererAdapter::onRendererAdded(MafwRegistry* mafw_registry,
										  GObject* renderer,
										  gpointer user_data)
{
  if(g_strcmp0(mafw_extension_get_name(MAFW_EXTENSION(renderer)), MEDIAPLAYER_RENDERER) == 0)
  {
    g_object_ref(renderer);
    static_cast<MafwRendererAdapter*>(user_data)->mafw_renderer = MAFW_RENDERER(renderer);
    static_cast<MafwRendererAdapter*>(user_data)->connectRendererSignals();
  }
}


void MafwRendererAdapter::onRendererRemoved(MafwRegistry* mafw_registry,
											GObject* renderer,
											gpointer user_data)
{
  if(g_strcmp0(mafw_extension_get_name(MAFW_EXTENSION(renderer)), MEDIAPLAYER_RENDERER) == 0)
  {
    g_object_unref(renderer);
    static_cast<MafwRendererAdapter*>(user_data)->mafw_renderer = MAFW_RENDERER(renderer);
    static_cast<MafwRendererAdapter*>(user_data)->disconnectRendererSignals();
  }
}

void MafwRendererAdapter::onBufferingInfo(MafwRenderer* mafw_renderer,
										  gfloat status,
										  gpointer user_data)
{
  g_print("on buffering info\n");
  emit static_cast<MafwRendererAdapter*>(user_data)->bufferingInfo(status);
}

void MafwRendererAdapter::onMediaChanged(MafwRenderer* mafw_renderer,
										 gint index,
										 gchar* object_id,
										 gpointer user_data)
{
  g_print("on media changed\n");
  emit static_cast<MafwRendererAdapter*>(user_data)->mediaChanged(index, object_id);
}

void MafwRendererAdapter::onMetadataChanged(MafwRenderer* mafw_renderer,
											gchar* name,
											GValueArray* value,
											gpointer user_data)
{
  g_print("on metadata changed %s\n", name);
  if(value->n_values == 1)
  {
    GValue* v = g_value_array_get_nth(value, 0);
    switch(G_VALUE_TYPE(v))
    {
    case G_TYPE_STRING:
      {
	const gchar* str_value = g_value_get_string(v);
	QVariant data = QVariant(str_value);
	emit static_cast<MafwRendererAdapter*>(user_data)->metadataChanged(QString(name), data);
      }
      break;
    case G_TYPE_INT:
      int int_value = g_value_get_int(v);
      QVariant data = QVariant(int_value);
      emit static_cast<MafwRendererAdapter*>(user_data)->metadataChanged(QString(name), data);
      break;
    } 
  }
}

void MafwRendererAdapter::onPlaylistChanged(MafwRenderer* renderer,
											GObject* playlist,
											gpointer user_data)
{
  g_print("on playlist changed\n");
  emit static_cast<MafwRendererAdapter*>(user_data)->playlistChanged(playlist);
}

void MafwRendererAdapter::onStateChanged(MafwRenderer* renderer,
										 gint state,
										 gpointer user_data)
{
  g_print("on state changed\n");
  emit static_cast<MafwRendererAdapter*>(user_data)->stateChanged(state);
}

void MafwRendererAdapter::play()
{
  if(mafw_renderer)
  {
    mafw_renderer_play(mafw_renderer, MafwRendererSignalHelper::play_playback_cb, this);
  }
}

void MafwRendererAdapter::playObject(const gchar* object_id)
{
  if(mafw_renderer)
  {
    mafw_renderer_play_object(mafw_renderer, object_id, MafwRendererSignalHelper::play_object_playback_cb, this);
  }
}

void MafwRendererAdapter::playURI(const gchar* uri)
{
  if(mafw_renderer)
  {
    mafw_renderer_play_uri(mafw_renderer, uri, MafwRendererSignalHelper::play_uri_playback_cb, this);
  }
}

void MafwRendererAdapter::stop()
{
  if(mafw_renderer)
  {
    mafw_renderer_stop(mafw_renderer, MafwRendererSignalHelper::stop_playback_cb, this);
  }
}


void MafwRendererAdapter::pause()
{
  if(mafw_renderer)
  {
    mafw_renderer_pause(mafw_renderer, MafwRendererSignalHelper::pause_playback_cb, this);
  }
}

void MafwRendererAdapter::resume()
{
  if(mafw_renderer)
  {
    mafw_renderer_resume(mafw_renderer, MafwRendererSignalHelper::resume_playback_cb, this);
  }
}

void MafwRendererAdapter::getStatus()
{
  if(mafw_renderer)
  {
    mafw_renderer_get_status(mafw_renderer, MafwRendererSignalHelper::get_status_cb, this);
  }
}

void MafwRendererAdapter::next()
{
  if(mafw_renderer)
  {
    mafw_renderer_next(mafw_renderer, MafwRendererSignalHelper::next_playback_cb, this);
  }
}

void MafwRendererAdapter::previous()
{
  if(mafw_renderer)
  {
    mafw_renderer_previous(mafw_renderer, MafwRendererSignalHelper::previous_playback_cb, this);
  }
}

void MafwRendererAdapter::gotoIndex(uint index)
{
  if(mafw_renderer)
  {
    mafw_renderer_goto_index(mafw_renderer, index, MafwRendererSignalHelper::goto_index_playback_cb, this);
  }
}

void MafwRendererAdapter::setPosition(MafwRendererSeekMode seekmode,
									  int seconds)
{
  if(mafw_renderer)
  {
    mafw_renderer_set_position(mafw_renderer, seekmode, seconds, MafwRendererSignalHelper::set_position_cb, this);
  }
}

void MafwRendererAdapter::getPosition()
{
  if(mafw_renderer)
  {
    mafw_renderer_get_position(mafw_renderer, MafwRendererSignalHelper::get_position_cb, this);
  }
}

void MafwRendererAdapter::getCurrentMetadata()
{
  if(mafw_renderer)
  {
    mafw_renderer_get_current_metadata(mafw_renderer, MafwRendererSignalHelper::get_current_metadata_cb, this);
  }
}

bool MafwRendererAdapter::assignPlaylist(MafwPlaylist* playlist)
{
  if(mafw_renderer)
  {
    return mafw_renderer_assign_playlist(mafw_renderer, playlist, NULL);
  }
  return false;
}
