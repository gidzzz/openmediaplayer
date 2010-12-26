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
    QDialog *freqDialog;
    QPushButton *freqButton;
    QListWidget *integers;
    QListWidget *fractions;

public:
    explicit FMTXDialog(QWidget *parent = 0);
    ~FMTXDialog();

protected:
    void showEvent(QShowEvent *event);

private slots:
    void showFreqDialog();
    void onSaveClicked();
};

#endif // FMTXDIALOG_H
