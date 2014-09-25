#ifndef FMTXDIALOG_H
#define FMTXDIALOG_H

#include <QDialog>
#include <QKeyEvent>

#include "ui_fmtxdialog.h"
#include "fmtxinterface.h"

class FMTXDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FMTXDialog(QWidget *parent = 0);
    ~FMTXDialog();

private:
    Ui::FMTXDialog *ui;

    FMTXInterface *fmtx;

    void keyPressEvent(QKeyEvent *e);
    void showError(const QString &message);

private slots:
    void onOrientationChanged(int w, int h);
    void onStateToggled(bool enabled);
    void accept();
};

#endif // FMTXDIALOG_H
