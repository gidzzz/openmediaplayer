#ifndef VIDEOSWINDOW_H
#define VIDEOSWINDOW_H

#include <QMainWindow>
#include <QActionGroup>
#include <QAction>

#include "ui_videoswindow.h"

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

private slots:
};

#endif // VIDEOSWINDOW_H
