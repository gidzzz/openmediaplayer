#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QMainWindow>

#include <QDebug>

#include <QShortcut>
#include <QMenuBar>
#include <QMenu>

/*#include "includes.h"
#include "rotator.h"*/

class BaseWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BaseWindow(QWidget *parent = 0);

private:
    /*void closeChildren();*/

private slots:
    /*void orientationChanged(int w, int h);*/
    void showWindowMenu();
};

#endif // BASEWINDOW_H
