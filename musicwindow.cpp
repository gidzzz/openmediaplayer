#include "musicwindow.h"
#include "ui_musicwindow.h"

MusicWindow::MusicWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MusicWindow)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    QMainWindow::setCentralWidget(ui->verticalLayoutWidget);
    QMainWindow::setWindowTitle(tr("Songs"));
    connect(ui->songList, SIGNAL(clicked(QModelIndex)), this, SLOT(selectSong()));
    myNowPlayingWindow = new NowPlayingWindow(this);
/*    QDir myDir("/home/user/MyDocs/.sounds/Music/Bullet for My Valentine/Fever");
    QStringList filters;
    filters << "*.mp3" << "*.acc" << "*.wav";
    myDir.setNameFilters(filters);
    QStringList list = myDir.entryList();*/
    listSongs();
}

MusicWindow::~MusicWindow()
{
    delete ui;
}

void MusicWindow::selectSong()
{
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
