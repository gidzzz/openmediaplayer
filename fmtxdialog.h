#ifndef FMTXDIALOG_H
#define FMTXDIALOG_H

#include <QDialog>
#include "ui_fmtxdialog.h"
#ifdef Q_WS_MAEMO_5
#include <QMaemo5ListPickSelector>
#include <QMaemo5InformationBox>
#include <QMaemo5ValueButton>
#include <QAbstractListModel>
#include <QList>
#include <QListView>
#endif

namespace Ui {
    class FMTXDialog;
}

class FMTXDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FMTXDialog(QWidget *parent = 0);
    ~FMTXDialog();

private:
    Ui::FMTXDialog *ui;
#ifdef Q_WS_MAEMO_5
    QMaemo5ValueButton *fmtxList;
    QMaemo5ListPickSelector *fmtxSelector;
    QListView *fmtxItems;
#endif

private slots:
    void onSaveClicked();
};

#endif // FMTXDIALOG_H
