#ifndef MAFWSOURCEADAPTER_H
#define MAFWSOURCEADAPTER_H

#include <QObject>

#include <QStringList>

#include <libmafw/mafw.h>

class MafwSourceAdapter : public QObject
{
    Q_OBJECT

public:
    MafwSourceAdapter(MafwSource *source);
    MafwSourceAdapter(const QString &uuid);
    ~MafwSourceAdapter();

    void bind(MafwSource *source);

    bool isReady();
    QString uuid();
    QString name();

    // Exposed operations
    uint browse(const QString &objectId, bool recursive, const char *filter, const char *sortCriteria, const char *const *metadataKeys, uint skipCount, uint itemCount);
    bool cancelBrowse(uint browseId);
    void getMetadata(const QString &objectId, const char *const *metadataKeys);
    void getUri(const QString &objectId);
    void createObject(const QString &parent, GHashTable *metadata);
    void destroyObject(const QString &objectId);
    void setMetadata(const QString &objectId, GHashTable *metadata);
    QString createObjectId(const QString &uri);

signals:
    // Exposed signals
    void containerChanged(const QString &objectId);
    void metadataChanged(const QString &objectId);
    void updating(int progress, int processedItems, int remainingItems, int remainingTime);

    // Exposed callbacks
    void browseResult(uint browseId, int remainingCount, uint index, const QString &objectId, GHashTable *metadata, const QString &error);
    void metadataResult(const QString &objectId, GHashTable *metadata, const QString &error);
    void gotUri(const QString &ojbectId, const QString &uri, const QString &error);
    void objectCreated(const QString &objectId, const QString &error);
    void objectDestroyed(const QString &objectId, const QString &error);
    void metadataSet(const QString &objectId, const QStringList &failedKeys, const QString &error);

private:
    MafwSource* source;
    QString m_uuid;

    void init();

    // Track active instances to avoid segfault on callback to a deleted object
    static QSet<gpointer> instances;

    // Signal handlers
    static void onContainerChanged(MafwSource *, gchar *objectId, MafwSourceAdapter *self);
    static void onMetadataChanged (MafwSource *, gchar *objectId, MafwSourceAdapter *self);
    static void onUpdating(MafwSource *, gint progress, gint processedItems, gint remainingItems, gint remainingTime, MafwSourceAdapter *self);

    // Callbacks
    static void onBrowseResult(MafwSource *, guint browseId, gint remainingCount, guint index, const gchar *objectId, GHashTable *metadata, gpointer self, const GError *error);
    static void onMetadataResult(MafwSource *, const gchar *objectId, GHashTable *metadata, gpointer self, const GError *error);
    static void onUriResult(MafwSource *, const gchar *objectId, GHashTable *metadata, gpointer self, const GError *error);
    static void onObjectCreated(MafwSource *, const gchar *objectId, gpointer self, const GError *error);
    static void onObjectDestroyed(MafwSource *, const gchar *objectId, gpointer self, const GError *error);
    static void onMetadataSet(MafwSource *, const gchar *objectId, const char **failedKeys, gpointer self, const GError *error);

private slots:
    void onSourceAdded(MafwSource *source);
    void onSourceRemoved(MafwSource *source);
};

#endif // MAFWSOURCEADAPTER_H
