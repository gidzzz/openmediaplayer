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
    this->selectView();
    if(sortByDate->isChecked())
        QMainWindow::setWindowTitle(tr("Videos - latest"));
    else
        QMainWindow::setWindowTitle(tr("Videos - categories"));
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
    if(action == sortByDate) {
        QMainWindow::setWindowTitle(tr("Videos - latest"));
        QSettings().setValue("Videos/Sortby", "date");
    } else if(action == sortByCategory) {
        QMainWindow::setWindowTitle(tr("Videos - categories"));
        QSettings().setValue("Videos/Sortby", "caregory");
    }
}

void VideosWindow::selectView()
{
    if(QSettings().value("Videos/Sortby").toString() == "category")
        sortByCategory->setChecked(true);
    else
        sortByDate->setChecked(true);
}
