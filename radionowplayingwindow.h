#ifndef RADIONOWPLAYINGWINDOW_H
#define RADIONOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDesktopWidget>

#ifdef Q_WS_MAEMO_5
    #include "fmtxdialog.h"
#endif

#ifdef MAFW
    #include "mafwrendereradapter.h"
    #include "mafwsourceadapter.h"
    #include "mafwplaylistadapter.h"
#endif
#include "includes.h"

namespace Ui {
    class RadioNowPlayingWindow;
}

class RadioNowPlayingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RadioNowPlayingWindow(QWidget *parent = 0, MafwRendererAdapter* mra = 0, MafwSourceAdapter* msa = 0, MafwPlaylistAdapter* pls = 0);
    ~RadioNowPlayingWindow();

private:
    Ui::RadioNowPlayingWindow *ui;
    void connectSignals();
    void setIcons();
    QTimer *volumeTimer;
#ifdef MAFW
    MafwRendererAdapter *mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter *playlist;
#endif

private slots:
    void toggleVolumeSlider();
    void orientationChanged();
#ifdef Q_WS_MAEMO_5
    void showFMTXDialog();
#endif
    void onNextButtonPressed();
    void onPrevButtonPressed();
};

#endif // RADIONOWPLAYINGWINDOW_H
