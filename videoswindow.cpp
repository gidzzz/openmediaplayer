#include "videoswindow.h"
#include "ui_videoswindow.h"

VideosWindow::VideosWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideosWindow)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    QMainWindow::setCentralWidget(ui->verticalLayoutWidget);
    sortByActionGroup = new QActionGroup(this);
    sortByActionGroup->setExclusive(true);
    sortByDate = new QAction(tr("Date"), sortByActionGroup);
    sortByDate->setCheckable(true);
    sortByDate->setChecked(true);
    sortByCategory = new QAction(tr("Category"), sortByActionGroup);
    sortByCategory->setCheckable(true);
    this->menuBar()->addActions(sortByActionGroup->actions());
}

VideosWindow::~VideosWindow()
{
    delete ui;
}
