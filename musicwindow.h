#ifndef MUSICWINDOW_H
#define MUSICWINDOW_H

#include <QMainWindow>
#include <nowplayingwindow.h>

namespace Ui {
    class MusicWindow;
}

class MusicWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MusicWindow(QWidget *parent = 0);
    ~MusicWindow();

public slots:
    void selectSong();

private:
    Ui::MusicWindow *ui;

};

#endif // MUSICWINDOW_H
