#ifndef VIDEOSWINDOW_H
#define VIDEOSWINDOW_H

#include <QMainWindow>
#include <QActionGroup>
#include <QAction>
#include <QSettings>

#include "ui_videoswindow.h"
#include "videonowplayingwindow.h"

namespace Ui {
    class VideosWindow;
}

class VideosWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideosWindow(QWidget *parent = 0);
    ~VideosWindow();

private:
    Ui::VideosWindow *ui;
    QActionGroup *sortByActionGroup;
    QAction *sortByDate;
    QAction *sortByCategory;
    void connectSignals();
    void selectView();

private slots:
    void onVideoSelected();
    void onSortingChanged(QAction*);
    void orientationChanged();
};

#endif // VIDEOSWINDOW_H
