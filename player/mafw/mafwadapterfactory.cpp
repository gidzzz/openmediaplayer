#include "mafwadapterfactory.h"

#include <libmafw-shared/mafw-shared.h>

MafwAdapterFactory* MafwAdapterFactory::instance = NULL;

MafwAdapterFactory* MafwAdapterFactory::get()
{
    if (!instance) {
        instance = new MafwAdapterFactory();

        // Additional initialization
        instance->renderer = new MafwRendererAdapter();
        instance->playlist = new MafwPlaylistAdapter(instance, instance->renderer);
        instance->sources[Tracker] = new MafwSourceAdapter("localtagfs");
        instance->sources[Radio ]= new MafwSourceAdapter("iradiosource");
        instance->sources[Upnp] = new MafwSourceAdapter("upnpcontrolsource");

#ifdef MAFW_WORKAROUNDS
        instance->renderer->playlist = instance->playlist;
#endif
    }
    return instance;
}

MafwAdapterFactory::MafwAdapterFactory()
{
    registry = mafw_registry_get_instance();
    g_signal_connect(registry, "source-added"  , G_CALLBACK(&onSourceAdded)  , static_cast<void*>(this));
    g_signal_connect(registry, "source-removed", G_CALLBACK(&onSourceRemoved), static_cast<void*>(this));

    mafw_shared_init(registry, NULL);
}

MafwSource* MafwAdapterFactory::findSourceByUUID(const QString &uuid)
{
    return MAFW_SOURCE(mafw_registry_get_extension_by_uuid(registry, uuid.toUtf8()));
}

MafwRendererAdapter* MafwAdapterFactory::getRenderer()
{
    return renderer;
}

MafwPlaylistAdapter* MafwAdapterFactory::getPlaylist()
{
    return playlist;
}

MafwSourceAdapter* MafwAdapterFactory::getTrackerSource()
{
    return sources[Tracker];
}

MafwSourceAdapter* MafwAdapterFactory::getRadioSource()
{
    return sources[Radio];
}

MafwSourceAdapter* MafwAdapterFactory::getUpnpSource()
{
    return sources[Upnp];
}

//--- Signal handlers ----------------------------------------------------------

void MafwAdapterFactory::onSourceAdded(MafwRegistry *, MafwSource *source, MafwAdapterFactory *self)
{
    emit self->sourceAdded(source);
    emit self->sourceAdded(mafw_extension_get_uuid(MAFW_EXTENSION(source)),
                           mafw_extension_get_name(MAFW_EXTENSION(source)));
}

void MafwAdapterFactory::onSourceRemoved(MafwRegistry *, MafwSource *source, MafwAdapterFactory *self)
{
    emit self->sourceRemoved(source);
    emit self->sourceRemoved(mafw_extension_get_uuid(MAFW_EXTENSION(source)),
                             mafw_extension_get_name(MAFW_EXTENSION(source)));
}
