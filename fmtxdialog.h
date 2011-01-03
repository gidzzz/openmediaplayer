#ifndef FMTXDIALOG_H
#define FMTXDIALOG_H

#include <QtGui>
#include "ui_fmtxdialog.h"
#include "freqpickselector.h"
#include <QtMaemo5>

namespace Ui {
    class FMTXDialog;
}

class FMTXDialog : public QDialog
{
    Q_OBJECT
    Ui::FMTXDialog *ui;
    QPushButton *freqButton;
    FreqPickSelector *selector;

public:
    explicit FMTXDialog(QWidget *parent = 0);
    ~FMTXDialog();

protected:
    void showEvent(QShowEvent *event);

private slots:
    void onSaveClicked();
};

#endif // FMTXDIALOG_H
