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
#include "nowplayingindicator.h"
#ifdef Q_WS_MAEMO_5
    #include <QMaemo5InformationBox>
    #include "fmtxdialog.h"
#endif

#include "includes.h"
#include "confirmdialog.h"
#include "radionowplayingwindow.h"
#include "bookmarkdialog.h"
#include "delegates/songlistitemdelegate.h"

#ifdef MAFW
    #include "mafw/mafwregistryadapter.h"
#endif

class InternetRadioWindow : public BrowserWindow
{
    Q_OBJECT

public:
    explicit InternetRadioWindow(QWidget *parent = 0, MafwRegistryAdapter *mafwRegistry = 0);

private:
    QList<QStandardItem*> audioBufferList;
    QList<QStandardItem*> videoBufferList;
#ifdef MAFW
    MafwRegistryAdapter *mafwRegistry;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwRadioSource;
    MafwPlaylistAdapter* playlist;
    unsigned int browseId;
#endif

private slots:
    void showFMTXDialog();
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onStationSelected(QModelIndex index);
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
#ifdef MAFW
    void listStations();
    void browseAllStations(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
#endif
};

#endif // INTERNETRADIOWINDOW_H
