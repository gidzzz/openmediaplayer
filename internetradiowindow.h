#ifndef INTERNETRADIOWINDOW_H
#define INTERNETRADIOWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSpacerItem>
#include <QMessageBox>
#include <QDialogButtonBox>
#include "nowplayingindicator.h"
#ifdef Q_WS_MAEMO_5
    #include <QMaemo5InformationBox>
    #include "fmtxdialog.h"
#endif

#include "ui_internetradiowindow.h"
#include "includes.h"
#include "headerawareproxymodel.h"
#include "radionowplayingwindow.h"
#include "bookmarkdialog.h"
#include "delegates/songlistitemdelegate.h"

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#endif

namespace Ui {
    class InternetRadioWindow;
}

class InternetRadioWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit InternetRadioWindow(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~InternetRadioWindow();
    bool eventFilter(QObject *, QEvent *e);

private:
    Ui::InternetRadioWindow *ui;
    void connectSignals();
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

    QStandardItemModel *stationModel;
    QSortFilterProxyModel *stationProxyModel;
    QList<QStandardItem*> audioBufferList;
    QList<QStandardItem*> videoBufferList;
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
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
    void onContextMenuRequested(const QPoint &point);
    void orientationChanged(int w, int h);
    void onChildClosed();
    void onSearchHideButtonClicked();
    void onSearchTextChanged(QString text);
#ifdef MAFW
    void listStations();
    void browseAllStations(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void onContainerChanged();
#endif
};

#endif // INTERNETRADIOWINDOW_H
