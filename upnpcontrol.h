#ifndef UPNPCONTROL_H
#define UPNPCONTROL_H

#include <QMainWindow>

#include "ui_upnpcontrol.h"
#include "includes.h"
#include "upnpview.h"

#include "mafw/mafwadapterfactory.h"

namespace Ui {
    class UpnpControl;
}

class UpnpControl : public QWidget
{
    Q_OBJECT

public:
    explicit UpnpControl(QWidget *parent = 0, MafwAdapterFactory *factory = 0);
    ~UpnpControl();

private slots:
    void onSourceAdded(QString uuid);
    void onSourceRemoved(QString uuid);
    void onItemActivated(QListWidgetItem *item);

private:
    Ui::UpnpControl *ui;
    MafwAdapterFactory *mafwFactory;
    MafwSourceAdapter *mafwUpnpSource;
    QStringList sources;
};

#endif // UPNPCONTROL_H
