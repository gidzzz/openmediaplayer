#ifndef VIDEOSWINDOW_H
#define VIDEOSWINDOW_H

#include <QMainWindow>
#include <QActionGroup>
#include <QAction>
#include <QSettings>

#include "ui_videoswindow.h"
#include "videonowplayingwindow.h"

#ifdef MAFW
    #include "mafwrendereradapter.h"
    #include "mafwsourceadapter.h"
    #include <libmafw/mafw-source.h>
#endif

namespace Ui {
    class VideosWindow;
}

class VideosWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideosWindow(QWidget *parent = 0, MafwSourceAdapter* msa = 0);
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
    MafwSourceAdapter *mafwTrackerSource;
    unsigned int browseAllVideosId;
#endif

private slots:
    void onVideoSelected();
    void onSortingChanged(QAction*);
    void orientationChanged();
#ifdef MAFW
    void listVideos();
    void browseAllVideos(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
#endif
};

#endif // VIDEOSWINDOW_H
