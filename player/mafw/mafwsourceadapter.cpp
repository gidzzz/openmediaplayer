#include "mafwsourceadapter.h"

#include "mafwregistryadapter.h"

QSet<gpointer> MafwSourceAdapter::instances;

MafwSourceAdapter::MafwSourceAdapter(MafwSource *source)
{
    init();

    bind(source);
}

MafwSourceAdapter::MafwSourceAdapter(const QString &uuid) :
    m_uuid(uuid)
{
    init();

    bind(MafwRegistryAdapter::get()->findSourceByUUID(uuid));
}

MafwSourceAdapter::~MafwSourceAdapter()
{
    instances.remove(this);

    bind(NULL);
}

void MafwSourceAdapter::init()
{
    instances.insert(this);

    source = NULL;

    connect(MafwRegistryAdapter::get(), SIGNAL(sourceAdded(MafwSource*)), this, SLOT(onSourceAdded(MafwSource*)));
}

void MafwSourceAdapter::bind(MafwSource *source)
{
    // Check if there is anything to do
    if (source == this->source)
        return;

    if (source) {
        // Unbind current source, if set, before proceeding
        if (this->source)
            bind(NULL);

        // Bind
        g_object_ref(source);
        g_signal_connect(source, "container-changed", G_CALLBACK(&onContainerChanged), static_cast<void*>(this));
        g_signal_connect(source, "metadata-changed" , G_CALLBACK(&onMetadataChanged) , static_cast<void*>(this));
        g_signal_connect(source, "updating"         , G_CALLBACK(&onUpdating)        , static_cast<void*>(this));

        this->source = source;
        m_uuid = mafw_extension_get_uuid(MAFW_EXTENSION(source));

        // Watch only for removals
        disconnect(MafwRegistryAdapter::get(), SIGNAL(sourceAdded(MafwSource*)), this, SLOT(onSourceAdded(MafwSource*)));
        connect(MafwRegistryAdapter::get(), SIGNAL(sourceRemoved(MafwSource*)), this, SLOT(onSourceRemoved(MafwSource*)));

        // Emit the signal in case something changed before the adapter got
        // reconnected after losing its source or to notify listeners about
        // readiness if this is the first time.
        emit containerChanged(uuid() + "::");
    } else {
        // Unbind
        g_signal_handlers_disconnect_matched(this->source, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, this);
        g_object_unref(this->source);

        this->source = NULL;
        m_uuid = QString();

        // Watch only for additions
        disconnect(MafwRegistryAdapter::get(), SIGNAL(sourceRemoved(MafwSource*)), this, SLOT(onSourceRemoved(MafwSource*)));
        connect(MafwRegistryAdapter::get(), SIGNAL(sourceAdded(MafwSource*)), this, SLOT(onSourceAdded(MafwSource*)));
    }
}

QString MafwSourceAdapter::uuid()
{
    return m_uuid;
}

QString MafwSourceAdapter::name()
{
    return source ? mafw_extension_get_name(MAFW_EXTENSION(source)) : QString();
}

bool MafwSourceAdapter::isReady()
{
    return source;
}

void MafwSourceAdapter::onSourceAdded(MafwSource *source)
{
    if (m_uuid == mafw_extension_get_uuid(MAFW_EXTENSION(source))) {
        // The control source must be activated for individual shares to appear
        if (m_uuid == "upnpcontrolsource")
            mafw_extension_set_property_boolean(MAFW_EXTENSION(source), "activate", true);

        bind(source);
    }
}

void MafwSourceAdapter::onSourceRemoved(MafwSource *source)
{
    if (m_uuid == mafw_extension_get_uuid(MAFW_EXTENSION(source)))
        bind(NULL);
}

//--- Operations ---------------------------------------------------------------

uint MafwSourceAdapter::browse(const QString &objectId, bool recursive, const char *filterString, const char *sortCriteria, const char *const *metadataKeys, uint skipCount, uint itemCount)
{
    if (source) {
        MafwFilter *filter = mafw_filter_parse(filterString);
        uint browseId = mafw_source_browse(source, objectId.toUtf8(), recursive, filter, sortCriteria, metadataKeys, skipCount, itemCount, &onBrowseResult, this);
        mafw_filter_free(filter);

        return browseId;
    } else {
        return MAFW_SOURCE_INVALID_BROWSE_ID;
    }
}

bool MafwSourceAdapter::cancelBrowse(uint browseId)
{
    return mafw_source_cancel_browse(source, browseId, NULL);
}

void MafwSourceAdapter::getMetadata(const QString &objectId, const char *const *metadataKeys)
{
    if (source)
        mafw_source_get_metadata(source, objectId.toUtf8(), metadataKeys, &onMetadataResult, this);
}

void MafwSourceAdapter::getUri(const QString &objectId)
{
    if (source)
        mafw_source_get_metadata(source, objectId.toUtf8(), MAFW_SOURCE_LIST(MAFW_METADATA_KEY_URI), &onUriResult, this);
}

void MafwSourceAdapter::createObject(const QString &parent, GHashTable *metadata)
{
    if (source)
        mafw_source_create_object(source, parent.toUtf8(), metadata, &onObjectCreated, this);
}

void MafwSourceAdapter::destroyObject(const QString &objectId)
{
    if (source)
        mafw_source_destroy_object(source, objectId.toUtf8(), &onObjectDestroyed, this);
}

void MafwSourceAdapter::setMetadata(const QString &objectId, GHashTable *metadata)
{
    if (source)
        mafw_source_set_metadata(source, objectId.toUtf8(), metadata, &onMetadataSet, this);
}

QString MafwSourceAdapter::createObjectId(const QString &uri)
{
    return QString::fromUtf8(mafw_source_create_objectid(uri.toUtf8()));
}

//--- Signal handlers ----------------------------------------------------------

void MafwSourceAdapter::onContainerChanged(MafwSource *, gchar *objectId, MafwSourceAdapter *self)
{
    emit self->containerChanged(QString::fromUtf8(objectId));
}

void MafwSourceAdapter::onMetadataChanged(MafwSource *, gchar *objectId, MafwSourceAdapter *self)
{
    emit self->metadataChanged(QString::fromUtf8(objectId));
}

void MafwSourceAdapter::onUpdating(MafwSource *, gint progress, gint processedItems, gint remainingItems, gint remainingTime, MafwSourceAdapter *self)
{
    emit self->updating(progress, processedItems, remainingItems, remainingTime);
}

//--- Callbacks ----------------------------------------------------------------

void MafwSourceAdapter::onBrowseResult(MafwSource *, guint browseId, gint remainingCount, guint index, const gchar *objectId, GHashTable *metadata, gpointer self, const GError *error)
{
    if (instances.contains(self))
        emit static_cast<MafwSourceAdapter*>(self)->browseResult(browseId, remainingCount, index, QString::fromUtf8(objectId), metadata, error ? error->message : QString());
}

void MafwSourceAdapter::onMetadataResult(MafwSource *, const gchar *objectId, GHashTable *metadata, gpointer self, const GError *error)
{
    if (instances.contains(self))
        emit static_cast<MafwSourceAdapter*>(self)->metadataResult(QString::fromUtf8(objectId), metadata, error ? error->message : QString());
}

void MafwSourceAdapter::onUriResult(MafwSource *, const gchar *objectId, GHashTable *metadata, gpointer self, const GError *error)
{
    if (instances.contains(self)) {
        QString uri;
        if (GValue *v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_URI))
            uri = QString::fromUtf8(g_value_get_string(v));

        emit static_cast<MafwSourceAdapter*>(self)->gotUri(QString::fromUtf8(objectId), uri, error ? error->message : QString());
    }
}

void MafwSourceAdapter::onObjectCreated(MafwSource *, const gchar* objectId, gpointer self, const GError *error)
{
    if (instances.contains(self))
        emit static_cast<MafwSourceAdapter*>(self)->objectCreated(QString::fromUtf8(objectId), error ? error->message : QString());
}

void MafwSourceAdapter::onObjectDestroyed(MafwSource *, const gchar *objectId, gpointer self, const GError *error)
{
    if (instances.contains(self))
        emit static_cast<MafwSourceAdapter*>(self)->objectDestroyed(QString::fromUtf8(objectId), error ? error->message : QString());
}

void MafwSourceAdapter::onMetadataSet(MafwSource *, const gchar *objectId, const gchar **failedKeys, gpointer self, const GError *error)
{
    if (instances.contains(self)) {
        QStringList failedKeyList;
        if (failedKeys)
            for (; *failedKeys; failedKeys++)
                failedKeyList.append(*failedKeys);

        emit static_cast<MafwSourceAdapter*>(self)->metadataSet(QString::fromUtf8(objectId), failedKeyList, error ? error->message : QString());
    }
}
