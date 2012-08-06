#ifndef SHAREDIALOG_H
#define SHAREDIALOG_H

#include <QDialog>
#include <QtCore>
#include <QKeyEvent>

#include "ui_sharedialog.h"

namespace Ui {
    class ShareDialog;
}

class ShareDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShareDialog(QWidget *parent = 0, QStringList files = QStringList());
    ~ShareDialog();

    QStringList files;

private:
    Ui::ShareDialog *ui;

    void keyPressEvent(QKeyEvent *e);

private slots:
    void sendViaBluetooth();
    void sendViaEmail();

};

#endif // SHAREDIALOG_H
