#ifndef _MAFW_RENDERER_SIGNAL_HELPER_HPP_
#define _MAFW_RENDERER_SIGNAL_HELPER_HPP_

#include <Qt>

#include <libmafw/mafw.h>
#include <libmafw-shared/mafw-shared.h>

#include "mafwrendereradapter.h"

class MafwRendererSignalHelper
{
 public:
  static void play_playback_cb(MafwRenderer* mafw_renderer, gpointer user_data, const GError* error);
  static void play_object_playback_cb(MafwRenderer* mafw_renderer, gpointer user_data, const GError* error);
  static void play_uri_playback_cb(MafwRenderer* mafw_renderer, gpointer user_data, const GError* error);
  static void stop_playback_cb(MafwRenderer* mafw_renderer, gpointer user_data, const GError* error);
  static void pause_playback_cb(MafwRenderer* mafw_renderer, gpointer user_data, const GError* error);
  static void resume_playback_cb(MafwRenderer* mafw_renderer, gpointer user_data, const GError* error);
  static void get_status_cb(MafwRenderer* mafw_renderer, MafwPlaylist* playlist, unsigned int index, MafwPlayState state, const char* object_id, gpointer user_data, const GError* error);
  static void next_playback_cb(MafwRenderer* mafw_renderer, gpointer user_data, const GError* error);
  static void previous_playback_cb(MafwRenderer* mafw_renderer, gpointer user_data, const GError* error);
  static void goto_index_playback_cb(MafwRenderer* mafw_renderer, gpointer user_data, const GError* error);
  static void set_position_cb(MafwRenderer* mafw_renderer, int position, gpointer user_data, const GError* error);
  static void get_position_cb(MafwRenderer* mafw_renderer, int position, gpointer user_data, const GError* error);
  static void get_current_metadata_cb(MafwRenderer* mafw_renderer, const gchar* object_id, GHashTable* metadata, gpointer user_data, const GError* error);
  static void get_property_cb(MafwExtension *self, const gchar *name, GValue *value, gpointer udata, const GError *error);
#ifdef MAFW_WORKAROUNDS
private:
  static int play_retries;
#endif
};
#endif
