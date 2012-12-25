#ifndef UPNPCONTROL_H
#define UPNPCONTROL_H

#include <QListWidget>

#include "includes.h"
#include "upnpview.h"

#include "mafw/mafwadapterfactory.h"

namespace Ui {
    class UpnpControl;
}

class UpnpControl : public QListWidget
{
    Q_OBJECT

public:
    explicit UpnpControl(QWidget *parent = 0, MafwAdapterFactory *factory = 0);

signals:
    void childOpened();
    void childClosed();

private slots:
    void onSourceAdded(QString uuid);
    void onSourceRemoved(QString uuid);
    void onItemActivated(QListWidgetItem *item);
    void onChildClosed();

private:
    MafwAdapterFactory *mafwFactory;
    MafwSourceAdapter *mafwUpnpSource;
    QStringList sources;
};

#endif // UPNPCONTROL_H
