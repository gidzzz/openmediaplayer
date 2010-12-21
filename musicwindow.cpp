#include "musicwindow.h"
#include "mafwrendereradapter.h"
#include "ui_musicwindow.h"

MusicWindow::MusicWindow(QWidget *parent, MafwRendererAdapter* mra) :
    QMainWindow(parent),
    ui(new Ui::MusicWindow),
    mafwrenderer(mra)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    shuffleAllButton = new QMaemo5ValueButton(this);
#else
    shuffleAllButton = new QPushButton(this);
#endif
    shuffleAllButton->setText(tr("Shuffle songs"));
    ui->verticalLayout->removeWidget(ui->songList);
    ui->verticalLayout->addWidget(shuffleAllButton);
    ui->verticalLayout->addWidget(ui->songList);
    QMainWindow::setCentralWidget(ui->verticalLayoutWidget);
    QMainWindow::setWindowTitle(tr("Songs"));
#ifdef Q_WS_MAEMO_5
    connect(ui->songList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectSong()));
#else
    connect(ui->songList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(selectSong()));
#endif
    myNowPlayingWindow = new NowPlayingWindow(this, mafwrenderer);
    listSongs();
#ifdef Q_WS_MAEMO_5
    int numberOfSongs = ui->songList->count();
    if(numberOfSongs == 1) {
        QString label = tr("song");
        shuffleAllButton->setValueText(QString::number(numberOfSongs) + " " + label);
    } else {
        QString label = tr("songs");
        shuffleAllButton->setValueText(QString::number(numberOfSongs) + " " + label);
    }
    shuffleAllButton->setValueLayout(QMaemo5ValueButton::ValueUnderTextCentered);;
#endif
}

MusicWindow::~MusicWindow()
{
    delete ui;
}

void MusicWindow::selectSong()
{
    myNowPlayingWindow->onMetadataChanged(ui->songList->currentRow()+1,
                                          ui->songList->count(),
                                          //QString("Song name"),
                                          QString(ui->songList->currentItem()->text()),
                                          tr("(unknown album)"),
                                          tr("(unknown arist)")
                                          );
    myNowPlayingWindow->show();
}

void MusicWindow::listSongs()
{
     QDirIterator directory_walker(
#ifdef Q_WS_MAEMO_5
                                   "/home/user/MyDocs/.sounds",
#else
                                   "/home",
#endif
                                   QDir::Files | QDir::NoSymLinks,
                                   QDirIterator::Subdirectories);

    while(directory_walker.hasNext())
    {
          directory_walker.next();

         if(directory_walker.fileInfo().completeSuffix() == "mp3")
                   ui->songList->addItem(directory_walker.fileName());
    }
}
