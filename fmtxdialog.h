#ifndef FMTXDIALOG_H
#define FMTXDIALOG_H

#include <QtGui>
#include <QtMaemo5>
#include <GConfItem>

#include "ui_fmtxdialog.h"
#include "freqpickselector.h"

namespace Ui {
    class FMTXDialog;
}

class FMTXDialog : public QDialog
{
    Q_OBJECT
    Ui::FMTXDialog *ui;
#ifdef Q_WS_MAEMO_5
    QMaemo5ValueButton *freqButton;
#else
    QPushButton *freqButton;
#endif
    FreqPickSelector *selector;
    GConfItem *fmtxState;
    GConfItem *fmtxFrequency;

public:
    explicit FMTXDialog(QWidget *parent = 0);
    ~FMTXDialog();

protected:
    void showEvent(QShowEvent *event);

private slots:
    void onSaveClicked();
    void onStateChanged();
};

#endif // FMTXDIALOG_H
