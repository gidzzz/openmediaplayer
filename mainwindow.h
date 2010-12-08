#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QDebug>
#include <QtGui>
#include <musicwindow.h>

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
    void paintEvent(QPaintEvent*);
    void setButtonIcons();
    void connectSignals();
    void setLabelText();

private slots:
    void showSongWindow();
    void orientationChanged();
};

#endif // MAINWINDOW_H
