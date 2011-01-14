#ifndef RADIONOWPLAYINGWINDOW_H
#define RADIONOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDesktopWidget>

#ifdef Q_WS_MAEMO_5
    #include "fmtxdialog.h"
#endif
#include "includes.h"

namespace Ui {
    class RadioNowPlayingWindow;
}

class RadioNowPlayingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RadioNowPlayingWindow(QWidget *parent = 0);
    ~RadioNowPlayingWindow();

private:
    Ui::RadioNowPlayingWindow *ui;
    void connectSignals();
    void setIcons();
    QTimer *volumeTimer;

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
