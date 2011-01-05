#ifndef NOWPLAYINGWINDOW_H
#define NOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtDBus>
#include <QTimer>

#include "mirror.h"
#include "cqgraphicsview.h"
#include "ui_nowplayingwindow.h"
#include "includes.h"
#include "delegates/playlistdelegate.h"

#ifdef Q_WS_MAEMO_5
    #include "mafwrendereradapter.h"
    #include "fmtxdialog.h"
#else
    class MafwRendererAdapter;
#endif
namespace Ui {
    class NowPlayingWindow;
}

class NowPlayingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NowPlayingWindow(QWidget *parent = 0, MafwRendererAdapter* mra = 0);
    ~NowPlayingWindow();
    void listSongs(QString);

public slots:
    void onMetadataChanged(int, int, QString, QString, QString);

private:
    Ui::NowPlayingWindow *ui;
#ifdef Q_WS_MAEMO_5
    MafwRendererAdapter* mafwrenderer;
#endif
    void setButtonIcons();
    void connectSignals();
    QTimer *volumeTimer;
    QGraphicsScene *albumArtScene;
    mirror *m;

private slots:
    void toggleVolumeSlider();
    void showFMTXDialog();
    void orientationChanged();
    void toggleList();
#ifdef Q_WS_MAEMO_5
    void onVolumeChanged(const QDBusMessage &msg);
#endif
    void stateChanged(int state);
    void metadataChanged(QString name, QVariant value);
    void volumeWatcher();
    void setAlbumImage(QString);
    void onShuffleButtonPressed();
    void onRepeatButtonPressed();
};

#endif // NOWPLAYINGWINDOW_H
