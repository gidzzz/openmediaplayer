#include "nowplayingwindow.h"
#include "ui_nowplayingwindow.h"

#define prevButtonIcon "/etc/hildon/theme/mediaplayer/Back.png"
#define playButtonIcon "/etc/hildon/theme/mediaplayer/Play.png"
#define nextButtonIcon "/etc/hildon/theme/mediaplayer/Forward.png"
#define repeatButtonIcon "/etc/hildon/theme/mediaplayer/Repeat.png"
#define repeatButtonPressedIcon "/etc/hildon/theme/mediaplayer/RepeatPressed.png"
#define shuffleButtonIcon "/etc/hildon/theme/mediaplayer/Shuffle.png"
#define shuffleButtonPressed "/etc/hildon/theme/mediaplayer/SufflePressed.png"
#define volumeButtonIcon "/usr/share/icons/hicolor/64x64/hildon/mediaplayer_volume.png"
#define albumImage "/usr/share/icons/hicolor/295x295/hildon/mediaplayer_default_album.png"

NowPlayingWindow::NowPlayingWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NowPlayingWindow)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    ui->volumeSlider->hide();
    ui->currentPositionLabel->setText("0:00");
    ui->trackLengthLabel->setText("0:00");
    ui->pushButton->setIcon(QIcon(albumImage));
    this->setButtonIcons();
    onMetadataChanged();
    ui->listView->hide();
    QMainWindow::setCentralWidget(ui->verticalLayoutWidget);
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(toggleVolumeSlider()));
    connect(ui->actionFM_Transmitter, SIGNAL(triggered()), this, SLOT(showFMTXDialog()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    listSongs();
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(toggleList()));
}

NowPlayingWindow::~NowPlayingWindow()
{
    delete ui;
}

void NowPlayingWindow::toggleVolumeSlider()
{
    if(ui->volumeSlider->isHidden()) {
        ui->prevButton->hide();
        ui->nextButton->hide();
        ui->playButton->hide();
        ui->shuffleButton->hide();
        ui->repeatButton->hide();
        ui->volumeSlider->show();
    } else {
        ui->prevButton->show();
        ui->nextButton->show();
        ui->playButton->show();
        ui->shuffleButton->show();
        ui->repeatButton->show();
        ui->volumeSlider->hide();
    }
}

void NowPlayingWindow::setButtonIcons()
{
    ui->prevButton->setIcon(QIcon(prevButtonIcon));
    ui->playButton->setIcon(QIcon(playButtonIcon));
    ui->nextButton->setIcon(QIcon(nextButtonIcon));
    ui->shuffleButton->setIcon(QIcon(shuffleButtonIcon));
    ui->repeatButton->setIcon(QIcon(repeatButtonIcon));
    ui->volumeButton->setIcon(QIcon(volumeButtonIcon));
}

void NowPlayingWindow::showFMTXDialog()
{
#ifdef Q_WS_MAEMO_5
    osso_cp_plugin_execute(osso_context, "/usr/lib/hildon-control-panel/libcpfmtx.so", NULL, TRUE);
#endif
}

void NowPlayingWindow::onMetadataChanged()
{
    ui->songNumberLabel->setText("226/9000");
    ui->songTitleLabel->setText("Song name");
    ui->albumNameLabel->setText("Album name");
    ui->artistLabel->setText("Artist name");
}

void NowPlayingWindow::orientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()) {
        // Landscape mode
        qDebug() << "NowPlayingWindow: Orientation changed: Landscape.";
        //if(ui->artworkLabel->isHidden())
            //ui->artworkLabel->show();
    } else {
        // Portrait mode
        qDebug() << "NowPlayingWindow: Orientation changed: Portrait.";
       // ui->artworkLabel->hide();
    }
}

void NowPlayingWindow::listSongs()
{
     QDirIterator directory_walker("/home/user/MyDocs/.sounds", QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);

    while(directory_walker.hasNext())
    {
          directory_walker.next();

         if(directory_walker.fileInfo().completeSuffix() == "mp3")
                   ui->listView->addItem(directory_walker.fileName());
    }
}

void NowPlayingWindow::toggleList()
{
    if(ui->listView->isHidden()) {
        ui->listView->show();
        ui->scrollArea->hide();
    } else {
        ui->listView->hide();
        ui->scrollArea->show();
    }
}

