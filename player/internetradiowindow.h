#ifndef INTERNETRADIOWINDOW_H
#define INTERNETRADIOWINDOW_H

#include "browserwindow.h"

#include <QDialog>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSpacerItem>
#include <QDialogButtonBox>
#include <QMaemo5InformationBox>

#include "includes.h"
#include "confirmdialog.h"
#include "metadatadialog.h"
#include "radionowplayingwindow.h"
#include "bookmarkdialog.h"
#include "fmtxdialog.h"
#include "delegates/songlistitemdelegate.h"

#include "mafw/mafwregistryadapter.h"

class InternetRadioWindow : public BrowserWindow
{
    Q_OBJECT

public:
    explicit InternetRadioWindow(QWidget *parent = 0, MafwRegistryAdapter *mafwRegistry = 0);

private:
    QList<QStandardItem*> audioBufferList;
    QList<QStandardItem*> videoBufferList;
    MafwRegistryAdapter *mafwRegistry;
    MafwRendererAdapter *mafwRenderer;
    MafwSourceAdapter *mafwRadioSource;
    CurrentPlaylistAdapter *playlist;
    unsigned int browseId;

private slots:
    void showFMTXDialog();
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onDetailsClicked();
    void onStationSelected(QModelIndex index);
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void listStations();
    void browseAllStations(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
};

#endif // INTERNETRADIOWINDOW_H
