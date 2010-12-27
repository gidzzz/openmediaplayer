#ifndef FMTXDIALOG_H
#define FMTXDIALOG_H

#include <QtGui>
#include "ui_fmtxdialog.h"

namespace Ui {
    class FMTXDialog;
}

class FMTXDialog : public QDialog
{
    Q_OBJECT
    Ui::FMTXDialog *ui;
    QPushButton *freqButton;

public:
    explicit FMTXDialog(QWidget *parent = 0);
    ~FMTXDialog();

protected:
    void showEvent(QShowEvent *event);

private slots:
    void onSaveClicked();
};

#endif // FMTXDIALOG_H
