#ifndef SHAREDIALOG_H
#define SHAREDIALOG_H

#include <QDialog>
#include <QDBusInterface>
#include <QKeyEvent>
#include <QTimer>

#include "ui_sharedialog.h"

#include "mafw/mafwsourceadapter.h"

namespace Ui {
    class ShareDialog;
}

class ShareDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShareDialog(QWidget *parent, MafwSourceAdapter *mafwSource, QString objectId);
    ~ShareDialog();

private:
    Ui::ShareDialog *ui;

    QString objectId;
    QString uri;

    void keyPressEvent(QKeyEvent *e);

    void (ShareDialog::*shareAction)();
    void shareViaBluetooth();
    void shareViaEmail();

private slots:
    void setProgressIndicator();
    void onUriReceived(QString objectId, QString uri);
    void onEmailClicked();
    void onBluetoothClicked();
};

#endif // SHAREDIALOG_H
