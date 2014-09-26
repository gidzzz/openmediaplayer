#include "pluginscontrol.h"

#include "pluginswindow.h"

PluginsControl::PluginsControl(QWidget *parent) :
    QAction(parent)
{
    this->setText(tr("Plug-ins"));
    this->setVisible(false);

    connect(MafwRegistryAdapter::get(), SIGNAL(sourceAdded(QString,QString)), this, SLOT(onSourceAdded(QString,QString)));
    connect(MafwRegistryAdapter::get(), SIGNAL(sourceRemoved(QString,QString)), this, SLOT(onSourceRemoved(QString)));

    connect(this, SIGNAL(triggered()), this, SLOT(openWindow()));
}

void PluginsControl::onSourceAdded(const QString &uuid, const QString &name)
{
    // Ignore UPnP shares and known standard sources
    if (uuid.startsWith("_uuid_") || MafwRegistryAdapter::get()->isRecognized(uuid)) return;

    sources[uuid] = name;

    this->setVisible(true);
}

void PluginsControl::onSourceRemoved(const QString &uuid)
{
    this->setVisible(sources.remove(uuid) && sources.isEmpty());
}

void PluginsControl::openWindow()
{
    emit childOpened();

    PluginsWindow *pluginsWindow = new PluginsWindow(this->parentWidget(), sources);
    pluginsWindow->show();

    connect(pluginsWindow, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
}

void PluginsControl::onChildClosed()
{
    emit childClosed();
}
