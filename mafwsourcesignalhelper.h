#ifndef _MAFW_SOURCE_SIGNAL_HELPER_HPP_
#define _MAFW_SOURCE_SIGNAL_HELPER_HPP_
#include <QStringList>
#include <libmafw/mafw.h>
#include <libmafw-shared/mafw-shared.h>

class MafwSourceSignalHelper
{
 public:

  static void browse_result_cb(MafwSource* mafw_source, 
			       guint browse_id, 
			       gint remaining_count, 
			       guint index, 
			       const gchar *object_id, 
			       GHashTable *metadata, 
			       gpointer user_data, 
			       const GError *error);

  static void metadata_result_cb(MafwSource* mafw_source, 
				 const char* object_id, 
				 GHashTable* metadata_keys, 
				 gpointer user_data, 
				 const GError* error);

  static void create_object_cb(MafwSource* mafw_source, 
			       const char* object_id, 
			       gpointer user_data, 
			       const GError* error);

  static void destroy_object_cb(MafwSource* mafw_source, 
				const char* object_id, 
				gpointer user_data, 
				const GError* error);
  
  static void set_metadata_cb(MafwSource* mafw_source,
			      const char* object_id,
			      const char** failed_keys,
			      gpointer user_data,
			      const GError* error);
};
#endif
