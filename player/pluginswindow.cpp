#include "pluginswindow.h"

#include "includes.h"
#include "upnpview.h"
#include "delegates/mediawithicondelegate.h"

PluginsWindow::PluginsWindow(QWidget *parent, const QHash<QString,QString> &sources) :
    BrowserWindow(parent, MafwRegistryAdapter::get())
{
    this->setWindowTitle(tr("Plug-ins"));

    ui->objectList->setItemDelegate(new MediaWithIconDelegate(ui->objectList));

    for (QHash<QString,QString>::const_iterator s = sources.begin(); s != sources.end(); ++s) {
        QStandardItem *item = new QStandardItem();

        item->setIcon(QIcon::fromTheme("mediaplayer_mafw_plugin"));
        item->setData(s.value(), UserRoleTitle);
        item->setData(s.key(), UserRoleObjectID);
        item->setData(Duration::Blank, UserRoleSongDuration);

        objectModel->appendRow(item);
    }

    objectProxyModel->setSortRole(UserRoleTitle);
    objectProxyModel->setSortLocaleAware(true);
    objectProxyModel->sort(0);

    connect(ui->objectList, SIGNAL(activated(QModelIndex)), this, SLOT(onItemActivated(QModelIndex)));
}

void PluginsWindow::onItemActivated(const QModelIndex &index)
{
    this->setEnabled(false);

    const QString uuid = index.data(UserRoleObjectID).toString();

    // UpnpView is universal enough to handle various types of sources
    UpnpView *upnpView = new UpnpView(this, MafwRegistryAdapter::get(), new MafwSourceAdapter(uuid));
    upnpView->browseObjectId(uuid + "::");
    upnpView->setWindowTitle(index.data(UserRoleTitle).toString());
    upnpView->show();

    connect(upnpView, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    ui->indicator->inhibit();
}
