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

#include <mafw/mafwsourceadapter.h>
#include <mafw/mafwsourcesignalhelper.h>

MafwSourceAdapter::MafwSourceAdapter(QString source):
    sourceIsReady(false),
    sourceName(source)
{
  mafw_registry = MAFW_REGISTRY(mafw_registry_get_instance());
  mafw_shared_init(mafw_registry, NULL);
  findSource();
  connectRegistrySignals();
}

MafwSourceAdapter::~MafwSourceAdapter()
{

}

bool
MafwSourceAdapter::isReady() const
{
  return sourceIsReady;
}

void
MafwSourceAdapter::findSource()
{
  if(mafw_registry)
  {
    GList* sources_list = mafw_registry_get_sources(mafw_registry);
    if(sources_list)
    {
      GList* source_elem = sources_list;
      while(source_elem)
      {
	MafwSource* mafw_source = MAFW_SOURCE(source_elem->data);
#ifdef DEBUG
	g_print("source: %s\n", mafw_extension_get_name(MAFW_EXTENSION(mafw_source)));
#endif
	if(sourceName.compare(mafw_extension_get_name(MAFW_EXTENSION(mafw_source))) == 0)
	{
#ifdef DEBUG
	  g_print("got source\n");
#endif
	  g_object_ref(mafw_source);
	  this->mafw_source = mafw_source;
	  this->sourceIsReady = true;
	  emit sourceReady();
	  connectSourceSignals();
	}
      }
    }
    else
    {
#ifdef DEBUG
      g_print("no source\n");
#endif
    }
  }
  else
  {
#ifdef DEBUG
    g_print("no rgistry\n");
#endif
  }
}

void
MafwSourceAdapter::connectRegistrySignals()
{
  g_signal_connect(mafw_registry,
		   "source_added",
		   G_CALLBACK(&onSourceAdded),
		   static_cast<void*>(this));

  g_signal_connect(mafw_registry,
		   "source_removed",
		   G_CALLBACK(&onSourceRemoved),
		   static_cast<void*>(this));
}

void
MafwSourceAdapter::onSourceAdded(MafwRegistry*,
				 GObject* source,
				 gpointer user_data)
{
  if(static_cast<MafwSourceAdapter*>(user_data)->sourceName.compare(mafw_extension_get_name(MAFW_EXTENSION(source))) == 0)
  {
    g_object_ref(source);
    static_cast<MafwSourceAdapter*>(user_data)->mafw_source = MAFW_SOURCE(source);
    static_cast<MafwSourceAdapter*>(user_data)->sourceIsReady = true;
    emit static_cast<MafwSourceAdapter*>(user_data)->sourceReady();
    static_cast<MafwSourceAdapter*>(user_data)->connectSourceSignals();
  }
}


void
MafwSourceAdapter::onSourceRemoved(MafwRegistry*,
				   GObject* source,
				   gpointer user_data)
{
  if(static_cast<MafwSourceAdapter*>(user_data)->sourceName.compare(mafw_extension_get_name(MAFW_EXTENSION(source))) == 0)
  {
    g_object_unref(source);
    static_cast<MafwSourceAdapter*>(user_data)->mafw_source = MAFW_SOURCE(source);
    static_cast<MafwSourceAdapter*>(user_data)->disconnectSourceSignals();
  }
}

void
MafwSourceAdapter::connectSourceSignals()
{
  g_signal_connect(mafw_source,
		   "container-changed",
		   G_CALLBACK(&onContainerChanged),
		   static_cast<void*>(this));
  g_signal_connect(mafw_source,
		   "metadata-changed",
		   G_CALLBACK(&onMetadataChanged),
		   static_cast<void*>(this));
  g_signal_connect(mafw_source,
		   "updating",
		   G_CALLBACK(&onUpdating),
		   static_cast<void*>(this));
}

void
MafwSourceAdapter::disconnectSourceSignals()
{

}

void
MafwSourceAdapter::onContainerChanged(MafwSource*, gchar* object_id, gpointer user_data)
{
#ifdef DEBUG
  g_print("on container changed %s\n", object_id);
#endif
  QString objectId(object_id);
  emit static_cast<MafwSourceAdapter*>(user_data)->containerChanged(objectId);
}

void
MafwSourceAdapter::onMetadataChanged(MafwSource*, gchar* object_id, gpointer user_data)
{
#ifdef DEBUG
  g_print("on metadata changed %s\n", object_id);
#endif
  QString objectId(object_id);
  emit static_cast<MafwSourceAdapter*>(user_data)->metadataChanged(objectId);
}

void
MafwSourceAdapter::onUpdating(MafwSource*, gint progress, gint processed_items, gint remaining_items, gint remaining_time, gpointer user_data)
{
#ifdef DEBUG
  g_print("on updating %d, %d\n", progress, remaining_items);
#endif
  emit static_cast<MafwSourceAdapter*>(user_data)->updating(progress, processed_items, remaining_items, remaining_time);
}

uint
MafwSourceAdapter::sourceBrowse(const char* object_id, bool recursive, const char* filterString, const char* sort_criteria, const char* const *metadata_keys, uint skip_count, uint item_count)
{
  uint rc = -1;
  MafwFilter *filter;
  if (filterString)
      filter = mafw_filter_parse (filterString);
  else
      filter = NULL;
  if(mafw_source)
  {
    rc = mafw_source_browse(mafw_source, object_id, recursive, filter, sort_criteria, metadata_keys, skip_count, item_count, MafwSourceSignalHelper::browse_result_cb, this);
  }
  return rc;
}

void
MafwSourceAdapter::getMetadata(const char* object_id, const char* const *metadata_keys)
{
  if(mafw_source)
  {
    mafw_source_get_metadata(mafw_source, object_id, metadata_keys, MafwSourceSignalHelper::metadata_result_cb, this);
  }
}

void
MafwSourceAdapter::getUri(const char* object_id)
{
  if(mafw_source)
  {
    mafw_source_get_metadata(mafw_source, object_id, MAFW_SOURCE_LIST(MAFW_METADATA_KEY_URI), MafwSourceSignalHelper::uri_result_cb, this);
  }
}

bool
MafwSourceAdapter::cancelBrowse(uint browseId,
				QString& qerror)
{
  bool rc = false;
  if(mafw_source)
  {
    GError* error = NULL;
    rc =  mafw_source_cancel_browse(mafw_source, browseId, &error);
    if(error)
    {
      qerror = error->message;
      g_error_free(error);
      error = NULL;
    }
  }
  return rc;
}

void
MafwSourceAdapter::createObject(const char* parent, GHashTable* metadata)
{
  if(mafw_source)
  {
    mafw_source_create_object(mafw_source, parent, metadata, MafwSourceSignalHelper::create_object_cb, this);
  }
}

void
MafwSourceAdapter::destroyObject(const char* object_id)
{
  if(mafw_source)
  {
    mafw_source_destroy_object(mafw_source, object_id, MafwSourceSignalHelper::destroy_object_cb, this);
  }
}


void
MafwSourceAdapter::setMetadata(const char* object_id,
			       GHashTable* metadata)
{
  if(mafw_source)
  {
    mafw_source_set_metadata(mafw_source, object_id, metadata, MafwSourceSignalHelper::set_metadata_cb, this);
  }
}

QString
MafwSourceAdapter::createObjectId(QString uri)
{
      return QString::fromUtf8(mafw_source_create_objectid (uri.toUtf8()));
}
