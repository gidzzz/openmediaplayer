/**************************************************************************
    This file is part of Open MediaPlayer
    Copyright (C) 2010-2011 Nicolai Hess

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

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


