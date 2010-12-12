#ifndef NOWPLAYINGWINDOW_H
#define NOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtDBus>
#ifdef Q_WS_MAEMO_5
#include <QLibrary>
#include <libosso.h>
#endif

namespace Ui {
    class NowPlayingWindow;
}

class NowPlayingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NowPlayingWindow(QWidget *parent = 0);
    ~NowPlayingWindow();

public slots:
    void onMetadataChanged(int, int, QString, QString, QString);

private:
    Ui::NowPlayingWindow *ui;
#ifdef Q_WS_MAEMO_5
    osso_context_t *osso_context;
#endif
    void setButtonIcons();
    void listSongs();
    void connectSignals();

private slots:
    void toggleVolumeSlider();
    void showFMTXDialog();
    void orientationChanged();
    void toggleList();
    void onVolumeChanged();
};

#endif // NOWPLAYINGWINDOW_H
