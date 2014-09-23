#include "upnpcontrol.h"

UpnpControl::UpnpControl(QWidget *parent) :
    QListWidget(parent)
{
    QFont font;
    font.setPointSize(13);
    this->setFont(font);

    this->setFlow(QListView::LeftToRight);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QColor c = QMaemo5Style::standardColor("ActiveTextColor");

    this->setStyleSheet(QString("QListWidget {background-color: transparent;}"
                                "QListWidget::item {background-color: transparent;}"
                                "QListWidget::item {selection-color: rgb(%1, %2, %3);}")
                                .arg(c.red()).arg(c.green()).arg(c.blue()));
}

void UpnpControl::setFactory(MafwAdapterFactory *factory)
{
    mafwFactory = factory;
    mafwUpnpSource = mafwFactory->getUpnpSource();

    connect(factory, SIGNAL(sourceAdded(QString,QString)), this, SLOT(onSourceAdded(QString,QString)));
    connect(factory, SIGNAL(sourceRemoved(QString,QString)), this, SLOT(onSourceRemoved(QString)));
    connect(this, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemActivated(QListWidgetItem*)));
}

void UpnpControl::onSourceAdded(const QString &uuid, const QString &name)
{
    qDebug() << "source added:" << uuid;

    if (uuid.startsWith("_uuid_") && !sources.contains(uuid)) {
        sources.append(uuid);

        QListWidgetItem *item = new QListWidgetItem();

        item->setIcon(QIcon::fromTheme("mediaplayer_upnp_server"));
        item->setText(name);
        item->setData(UserRoleObjectID, uuid);

        this->addItem(item);
    }
}

void UpnpControl::onSourceRemoved(const QString &uuid)
{
    qDebug() << "source removed:" << uuid;

    for (int i = 0; i < this->count(); i++) {
        if (this->item(i)->data(UserRoleObjectID).toString() == uuid) {
            delete this->item(i);
            break;
        }
    }

    sources.removeOne(uuid);
}

void UpnpControl::onItemActivated(QListWidgetItem *item)
{
    emit childOpened();

    QString uuid = item->data(UserRoleObjectID).toString();

    UpnpView *upnpView = new UpnpView(this, mafwFactory, new MafwSourceAdapter(uuid));
    upnpView->browseObjectId(uuid + "::");
    upnpView->setWindowTitle(item->text());
    upnpView->show();

    connect(upnpView, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
}

void UpnpControl::onChildClosed()
{
    this->clearSelection();
    emit childClosed();
}
