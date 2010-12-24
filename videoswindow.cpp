#include "videoswindow.h"

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
    this->connectSignals();
}

VideosWindow::~VideosWindow()
{
    delete ui;
}

void VideosWindow::connectSignals()
{
    connect(ui->listWidget, SIGNAL(activated(QModelIndex)), this, SLOT(onVideoSelected()));
    connect(ui->menubar, SIGNAL(triggered(QAction*)), this, SLOT(onSortingChanged(QAction*)));
}

void VideosWindow::onVideoSelected()
{
    // Placeholder function
    ui->listWidget->clearSelection();
}

void VideosWindow::onSortingChanged(QAction *action)
{
    if(action == sortByDate)
        QMainWindow::setWindowTitle(tr("Videos - latest"));
    else if(action == sortByCategory)
        QMainWindow::setWindowTitle(tr("Videos - categories"));
}
