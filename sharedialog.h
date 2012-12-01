#ifndef SHAREDIALOG_H
#define SHAREDIALOG_H

#include <QDialog>
#include <QDBusInterface>
#include <QUrl>
#include <QKeyEvent>

#include "ui_sharedialog.h"

namespace Ui {
    class ShareDialog;
}

class ShareDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShareDialog(QStringList files, QWidget *parent = 0);
    ~ShareDialog();

    QStringList files;

private:
    Ui::ShareDialog *ui;

    void keyPressEvent(QKeyEvent *e);

private slots:
    void onEmailClicked();
    void onBluetoothClicked();

};

#endif // SHAREDIALOG_H
