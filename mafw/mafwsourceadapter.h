#ifndef _MAFW_SOURCE_ADAPTER_HPP_
#define _MAFW_SOURCE_ADAPTER_HPP_

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <libmafw/mafw.h>
#include <libmafw-shared/mafw-shared.h>
#include <glib.h>

class MafwSourceAdapter : public QObject
{
  Q_OBJECT
  
    friend class MafwSourceSignalHelper;

 public:
  MafwSourceAdapter(QString sourceName);
  ~MafwSourceAdapter();
  
  bool isReady() const;
  static void onSourceAdded(MafwRegistry* mafw_registry, 
			    GObject* source, 
			    gpointer user_data);

  static void onSourceRemoved(MafwRegistry* mafw_registry, 
			      GObject* source,
			      gpointer user_data);
  
  static void onContainerChanged(MafwSource* mafw_source, 
				 gchar* object_id, 
				 gpointer user_data);

  static void onMetadataChanged(MafwSource* mafw_source, 
				gchar* object_id, 
				gpointer user_data);

  static void onUpdating(MafwSource* mafw_source, 
			 gint progress, 
			 gint processed_items, 
			 gint remaining_items, 
			 gint remaining_time, 
			 gpointer user_data);

  public slots:
  
  uint sourceBrowse(const char* object_id, 
		    bool  recursive, 
                    const char* filter,
		    const char* sort_criteria, 
		    const char* const *metadata_keys, 
		    uint skip_count, 
		    uint item_count);

  bool cancelBrowse(uint browseId,
		    QString& error);

  void getMetadata(const char* object_id, 
		   const char* const* metadata_keys);

  void getUri(const char* object_id);

  void createObject(const char* parent,
		    GHashTable* metadata);

  void destroyObject(const char* parent);
  
  void setMetadata(const char* object_id,
		   GHashTable* metadata);

  QString createObjectId(QString uri);
		    
 signals:
  //MafwSource signals
  void containerChanged(QString objectId);
  void metadataChanged(QString objectId);

  void updating(int progress, 
		int processed_items, 
		int remaining_items, 
		int remaining_time);

  void sourceReady();

  //Mafw callbacks as signals
  void signalSourceBrowseResult(uint browseId, 
				int remainingCount, 
				uint index, 
				QString objectId, 
				GHashTable* metadata_keys, 
				QString error);

  void signalMetadataResult(QString objectId, 
			    GHashTable* metadata_keys, 
			    QString error);

  void signalGotUri(QString ojbectId, QString Uri);
  
  void signalCreateObjectResult(QString objectId, QString error);
  void signalDestroyObjectResult(QString objectId, QString error);
  void signalMetadataSetResult(QString objectId, QStringList failed_keys, QString error);

 private:

  void findSource();
  void connectRegistrySignals();
  void connectSourceSignals();
  void disconnectSourceSignals();

  MafwRegistry* mafw_registry;
  MafwSource* mafw_source;
  bool sourceIsReady;
  QString sourceName;
};
#endif
