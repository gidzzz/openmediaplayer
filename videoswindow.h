#ifndef VIDEOSWINDOW_H
#define VIDEOSWINDOW_H

#include <QMainWindow>
#include <QActionGroup>
#include <QAction>
#include <QSettings>

#include "ui_videoswindow.h"
#include "videonowplayingwindow.h"
#include "delegates/thumbnailitemdelegate.h"

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

private:
    Ui::VideosWindow *ui;
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    QActionGroup *sortByActionGroup;
    QAction *sortByDate;
    QAction *sortByCategory;
    void connectSignals();
    void selectView();
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    unsigned int browseAllVideosId;
#endif

private slots:
    void onVideoSelected(QListWidgetItem*);
    void onSortingChanged(QAction*);
    void orientationChanged();
#ifdef MAFW
    void listVideos();
    void browseAllVideos(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void onContainerChanged(QString objectId);
#endif
};

#endif // VIDEOSWINDOW_H
