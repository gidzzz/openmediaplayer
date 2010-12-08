#ifndef VIDEOSWINDOW_H
#define VIDEOSWINDOW_H

#include <QMainWindow>

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
};

#endif // VIDEOSWINDOW_H
