#include "mafwsourcesignalhelper.h"
#include "mafwsourceadapter.h"
#include <QString>


void
MafwSourceSignalHelper::browse_result_cb(MafwSource* mafw_source, guint browse_id, gint remaining_count, guint index, const gchar *object_id, GHashTable *metadata, gpointer user_data, const GError *error)
{
  QString qerror;
  if(error)
  {
    qerror = QString(error->message);
  }
  emit static_cast<MafwSourceAdapter*>(user_data)->signalSourceBrowseResult(browse_id, remaining_count, index, QString(object_id), metadata, qerror);
}

void
MafwSourceSignalHelper::metadata_result_cb(MafwSource* mafw_source, const char* object_id, GHashTable* metadata_keys, gpointer user_data, const GError* error)
{
  QString qerror;
  if(error)
  {
    qerror = QString(error->message);
  }
  emit static_cast<MafwSourceAdapter*>(user_data)->signalMetadataResult(object_id, metadata_keys, qerror);
}

void
MafwSourceSignalHelper::create_object_cb(MafwSource* mafw_source, 
					 const char* object_id, 
					 gpointer user_data, 
					 const GError* error)
{
  QString qerror;
  if(error)
  {
    qerror = error->message;
  }
  emit static_cast<MafwSourceAdapter*>(user_data)->signalCreateObjectResult(object_id, qerror);
}

void
MafwSourceSignalHelper::destroy_object_cb(MafwSource* mafw_source, 
					  const char* object_id, 
					  gpointer user_data, 
					  const GError* error)
{
  QString qerror;
  if(error)
  {
    qerror = error->message;
  }
  emit static_cast<MafwSourceAdapter*>(user_data)->signalDestroyObjectResult(object_id, qerror);
}

void
MafwSourceSignalHelper::set_metadata_cb(MafwSource* mafw_source,
					const char* object_id,
					const char** failed_keys,
					gpointer user_data,
					const GError* error)
{
  QString qerror;
  if(error)
  {
    qerror = QString(error->message);
  }
  const char** failed_key = failed_keys;
  QStringList failed_key_list;
  while(failed_key)
  {
    failed_key_list.push_back(*failed_key);
    failed_key++;
  }
  
  emit static_cast<MafwSourceAdapter*>(user_data)->signalMetadataSetResult(object_id, 
									   failed_key_list,
									   qerror);
}


