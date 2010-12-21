#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QDebug>
#include <QtGui>
#include <musicwindow.h>
#include <videoswindow.h>
#include <internetradiowindow.h>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    MusicWindow *myMusicWindow;
    VideosWindow *myVideosWindow;
    InternetRadioWindow *myInternetRadioWindow;
    void paintEvent(QPaintEvent*);
    void setButtonIcons();
    void connectSignals();
    void setLabelText();

private slots:
    void showMusicWindow();
    void showVideosWindow();
    void orientationChanged();
    void showAbout();
    void processListClicks(QListWidgetItem*);
    void showInternetRadioWindow();
};

#endif // MAINWINDOW_H
