#include "upnpcontrol.h"

UpnpControl::UpnpControl(QWidget *parent, MafwAdapterFactory *factory) :
    QWidget(parent),
    ui(new Ui::UpnpControl),
    mafwFactory(factory)
{
    ui->setupUi(this);

    QColor c = QMaemo5Style::standardColor("ActiveTextColor");

    ui->upnpList->setStyleSheet(QString("QListWidget {background-color: transparent;}"
                                        "QListWidget::item {background-color: transparent;}"
                                        "QListWidget::item {selection-color: rgb(%1, %2, %3);}")
                                        .arg(c.red()).arg(c.green()).arg(c.blue()));

    mafwUpnpSource = mafwFactory->getUpnpSource();

    connect(mafwUpnpSource, SIGNAL(sourceAdded(QString)), this, SLOT(onSourceAdded(QString)));
    connect(mafwUpnpSource, SIGNAL(sourceRemoved(QString)), this, SLOT(onSourceRemoved(QString)));
    connect(ui->upnpList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemActivated(QListWidgetItem*)));
}

UpnpControl::~UpnpControl()
{
    delete ui;
}

void UpnpControl::onSourceAdded(QString uuid)
{
    qDebug() << "source added:" << uuid;

    if (uuid.startsWith("_uuid_") && !sources.contains(uuid)) {
        sources.append(uuid);

        QListWidgetItem *item = new QListWidgetItem(ui->upnpList);

        item->setIcon(QIcon::fromTheme("mediaplayer_upnp_server"));
        item->setText(mafwUpnpSource->getNameByUUID(uuid));
        item->setData(UserRoleObjectID, uuid);

        ui->upnpList->addItem(item);
    }
}

void UpnpControl::onSourceRemoved(QString uuid)
{
    qDebug() << "source removed:" << uuid;

    for (int i = 0; i < ui->upnpList->count(); i++) {
        if (ui->upnpList->item(i)->data(UserRoleObjectID).toString() == uuid) {
            delete ui->upnpList->item(i);
            break;
        }
    }

    sources.removeOne(uuid);
}

void UpnpControl::onItemActivated(QListWidgetItem *item)
{
    emit childOpened();

    QString uuid = item->data(UserRoleObjectID).toString();

    MafwSourceAdapter *source = new MafwSourceAdapter(mafwUpnpSource->getSourceByUUID(uuid));

    UpnpView *upnpView = new UpnpView(this, mafwFactory, source);
    upnpView->browseObjectId(uuid + "::");
    upnpView->setWindowTitle(item->text());
    upnpView->show();

    connect(upnpView, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
}

void UpnpControl::onChildClosed()
{
    ui->upnpList->clearSelection();
    emit childClosed();
}
