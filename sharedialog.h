#ifndef SHAREDIALOG_H
#define SHAREDIALOG_H

#include <QDialog>
#include <QtCore>

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

private slots:
    void on_share_mail_released();
    void on_share_bt_released();

};

#endif // SHAREDIALOG_H
