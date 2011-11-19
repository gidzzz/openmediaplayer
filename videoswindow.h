#ifndef VIDEOSWINDOW_H
#define VIDEOSWINDOW_H

#include <QMainWindow>
#include <QActionGroup>
#include <QAction>
#include <QSettings>

#include "ui_videoswindow.h"
#include "videonowplayingwindow.h"
#include "delegates/thumbnailitemdelegate.h"
#include "delegates/mediawithicondelegate.h"

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
    #include <libmafw/mafw-source.h>
#endif

namespace Ui {
    class VideosWindow;
}

class VideosWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideosWindow(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~VideosWindow();
    bool eventFilter(QObject *, QEvent *event);

private:
    Ui::VideosWindow *ui;
    QListWidget bufferList;
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    QActionGroup *sortByActionGroup;
    QAction *sortByDate;
    QAction *sortByCategory;
    void connectSignals();
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    unsigned int browseId;
#endif

private slots:
    void onShareClicked();
    void onDeleteClicked();
    void onContextMenuRequested(const QPoint &point);
    void onVideoSelected(QListWidgetItem*);
    void onSortingChanged(QAction*);
    void orientationChanged();
    void onChildClosed();
    void selectView();
#ifdef MAFW
    void listVideos();
    void browseAllVideos(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void onShareUriReceived(QString objectId, QString uri);
    void onMetadataChanged(QString metadata, QVariant value);
    void onSourceMetadataChanged(QString objectId);
    void onContainerChanged(QString objectId);
#endif
};

#endif // VIDEOSWINDOW_H
