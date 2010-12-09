#include "musicwindow.h"
#include "ui_musicwindow.h"

MusicWindow::MusicWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MusicWindow)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    shuffleAllButton = new QMaemo5ValueButton(this);
    shuffleAllButton->setText(tr("Shuffle songs"));
#else
    shuffleAllButton = new QPushButton(this);
#endif
    ui->verticalLayout->removeWidget(ui->songList);
    ui->verticalLayout->addWidget(shuffleAllButton);
    ui->verticalLayout->addWidget(ui->songList);
    QMainWindow::setCentralWidget(ui->verticalLayoutWidget);
    QMainWindow::setWindowTitle(tr("Songs"));
    connect(ui->songList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectSong()));
    myNowPlayingWindow = new NowPlayingWindow(this);
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
    myNowPlayingWindow->onMetadataChanged(ui->songList->currentRow()+1, ui->songList->count(), QString("Song name") , QString("Album Name"), QString("Artist Name"));
    myNowPlayingWindow->show();
}

void MusicWindow::listSongs()
{
     QDirIterator directory_walker("/home/user/MyDocs/.sounds", QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);

    while(directory_walker.hasNext())
    {
          directory_walker.next();

         if(directory_walker.fileInfo().completeSuffix() == "mp3")
                   ui->songList->addItem(directory_walker.fileName());
    }
}
