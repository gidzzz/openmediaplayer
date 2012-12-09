#ifndef OPENDIALOG_H
#define OPENDIALOG_H

#include <QDialog>
#include <QSettings>

#include "ui_opendialog.h"
#include "includes.h"

namespace Ui {
    class OpenDialog;
}

class OpenDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OpenDialog(QWidget *parent = 0);
    ~OpenDialog();

private:
    Ui::OpenDialog *ui;

private slots:
    void onButtonClicked();
};

#endif // OPENDIALOG_H
