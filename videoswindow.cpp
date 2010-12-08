#include "videoswindow.h"
#include "ui_videoswindow.h"

VideosWindow::VideosWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideosWindow)
{
    ui->setupUi(this);
}

VideosWindow::~VideosWindow()
{
    delete ui;
}
