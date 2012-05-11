#ifndef FMTXDIALOG_H
#define FMTXDIALOG_H

#include <QtGui>
#include <QtMaemo5>
#include <gq/GConfItem>
/* /usr/include/gq is provided by libgq-gconf-dev and libgq-gconf0
   These packages need to be installed on top of your sysrootfs.
         Join #maemo-foss on FreeNode for more information. */

#include "ui_fmtxdialog.h"
#include "freqdlg.h"

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
    FreqDlg *selector;
    GConfItem *fmtxState;
    GConfItem *fmtxFrequency;

public:
    explicit FMTXDialog(QWidget *parent = 0);
    ~FMTXDialog();

protected:
    void showEvent(QShowEvent *event);

private:
    void keyPressEvent(QKeyEvent *);
    void showErrorNote(QString error);

private slots:
    void showDialog();
    void onSaveClicked();
    void onStateChanged();
    void orientationChanged(int w, int h);
    void onCheckboxClicked();
};

#endif // FMTXDIALOG_H
