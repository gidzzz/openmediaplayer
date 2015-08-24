#include "mafwregistryadapter.h"

#include <libmafw-shared/mafw-shared.h>

MafwRegistryAdapter* MafwRegistryAdapter::instance = NULL;

MafwRegistryAdapter* MafwRegistryAdapter::get()
{
    if (!instance) {
        instance = new MafwRegistryAdapter();

        // Additional initialization
        instance->m_renderer = new MafwRendererAdapter("gstrenderer");
        instance->m_playlist = new CurrentPlaylistAdapter(instance->m_renderer, instance);
        instance->sources[Tracker] = new MafwSourceAdapter("localtagfs");
        instance->sources[Radio ]= new MafwSourceAdapter("iradiosource");
        instance->sources[Upnp] = new MafwSourceAdapter("upnpcontrolsource");

#ifdef MAFW_WORKAROUNDS
        instance->m_renderer->playlist = instance->m_playlist;
#endif
    }
    return instance;
}

MafwRegistryAdapter::MafwRegistryAdapter()
{
    registry = mafw_registry_get_instance();
    g_signal_connect(registry, "renderer-added"  , G_CALLBACK(&onRendererAdded)  , static_cast<void*>(this));
    g_signal_connect(registry, "renderer-removed", G_CALLBACK(&onRendererRemoved), static_cast<void*>(this));
    g_signal_connect(registry, "source-added"  , G_CALLBACK(&onSourceAdded)  , static_cast<void*>(this));
    g_signal_connect(registry, "source-removed", G_CALLBACK(&onSourceRemoved), static_cast<void*>(this));

    mafw_shared_init(registry, NULL);
}

MafwExtension* MafwRegistryAdapter::findExtensionByUuid(const QString &uuid)
{
    return mafw_registry_get_extension_by_uuid(registry, uuid.toUtf8());
}

MafwRendererAdapter* MafwRegistryAdapter::renderer()
{
    return m_renderer;
}

CurrentPlaylistAdapter* MafwRegistryAdapter::playlist()
{
    return m_playlist;
}

MafwSourceAdapter* MafwRegistryAdapter::source(RecognizedSource source)
{
    return sources[source];
}

bool MafwRegistryAdapter::isRecognized(const QString &uuid)
{
    for (int i = 0; i < RecognizedSourceCount; i++)
        if (sources[i]->uuid() == uuid)
            return true;
    return false;
}

//--- Signal handlers ----------------------------------------------------------

void MafwRegistryAdapter::onRendererAdded(MafwRegistry *, MafwRenderer *renderer, MafwRegistryAdapter *self)
{
    emit self->rendererAdded(renderer);
}

void MafwRegistryAdapter::onRendererRemoved(MafwRegistry *, MafwRenderer *renderer, MafwRegistryAdapter *self)
{
    emit self->rendererRemoved(renderer);
}

void MafwRegistryAdapter::onSourceAdded(MafwRegistry *, MafwSource *source, MafwRegistryAdapter *self)
{
    emit self->sourceAdded(source);
    emit self->sourceAdded(mafw_extension_get_uuid(MAFW_EXTENSION(source)),
                           mafw_extension_get_name(MAFW_EXTENSION(source)));
}

void MafwRegistryAdapter::onSourceRemoved(MafwRegistry *, MafwSource *source, MafwRegistryAdapter *self)
{
    emit self->sourceRemoved(source);
    emit self->sourceRemoved(mafw_extension_get_uuid(MAFW_EXTENSION(source)),
                             mafw_extension_get_name(MAFW_EXTENSION(source)));
}
