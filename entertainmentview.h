#ifndef ENTERTAINMENTVIEW_H
#define ENTERTAINMENTVIEW_H

#include <QMainWindow>
#include <QDeclarativeView>
#include <QGraphicsObject>
#include <QGLWidget>

#include "ui_entertainmentview.h"

namespace Ui {
    class EntertainmentView;
}

class EntertainmentView : public QMainWindow
{
    Q_OBJECT

public:
    explicit EntertainmentView(QWidget *parent = 0);
    ~EntertainmentView();
    void setMetadata(QString songName, QString albumName, QString artistName, QString albumArtUri);

signals:
    void titleChanged(QVariant);
    void albumChanged(QVariant);
    void artistChanged(QVariant);
    void albumArtChanged(QVariant);

private:
    Ui::EntertainmentView *ui;
    QVariant title;
    QVariant album;
    QVariant artist;
    QVariant albumArt;
    QObject *rootObject;
#ifdef Q_WS_MAEMO_5
    void setDNDAtom(bool dnd);
#endif
};

#endif // ENTERTAINMENTVIEW_H
