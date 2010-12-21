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

class MafwRendererAdapter;
class NowPlayingWindow : public QMainWindow
{
    Q_OBJECT

public:
  explicit NowPlayingWindow(QWidget *parent = 0, MafwRendererAdapter* mra = 0);
    ~NowPlayingWindow();

public slots:
    void onMetadataChanged(int, int, QString, QString, QString);

private:
    Ui::NowPlayingWindow *ui;
#ifdef Q_WS_MAEMO_5
    osso_context_t *osso_context;
    MafwRendererAdapter* mafwrenderer;

#endif
    void setButtonIcons();
    void listSongs();
    void connectSignals();

private slots:
    void toggleVolumeSlider();
    void showFMTXDialog();
    void orientationChanged();
    void toggleList();
#ifdef Q_WS_MAEMO_5
    void onVolumeChanged(const QDBusMessage &msg);
#endif
    void onVolumeChanged();
    void stateChanged(int state);
    void metadataChanged(QString name, QVariant value);
};

#endif // NOWPLAYINGWINDOW_H
