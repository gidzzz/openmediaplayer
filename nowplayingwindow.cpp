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
    //ui->volumeSlider_2->hide();
    ui->currentPositionLabel->setText("0:00");
    ui->trackLengthLabel->setText("0:00");
    ui->artworkButton->setIcon(QIcon(albumImage));
    ui->artworkButton_2->setIcon(ui->artworkButton->icon());
    this->setButtonIcons();
    //this->onMetadataChanged()
    ui->songPlaylist->hide();
    ui->songPlaylist_2->hide();
    QMainWindow::setCentralWidget(ui->horizontalWidget);
    ui->landscapeWidget->setLayout(ui->verticalLayout);
    ui->portraitWidget->setLayout(ui->verticalLayout_3);
    ui->portraitWidget->hide();
    this->connectSignals();
    this->listSongs();
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
    ui->prevButton_2->setIcon(QIcon(prevButtonIcon));
    ui->playButton_2->setIcon(QIcon(playButtonIcon));
    ui->nextButton_2->setIcon(QIcon(nextButtonIcon));
    ui->shuffleButton_2->setIcon(QIcon(shuffleButtonIcon));
    ui->repeatButton_2->setIcon(QIcon(repeatButtonIcon));
    //ui->volumeButton_2->setIcon(QIcon(volumeButtonIcon));
}

void NowPlayingWindow::connectSignals()
{
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(toggleVolumeSlider()));
    connect(ui->actionFM_Transmitter, SIGNAL(triggered()), this, SLOT(showFMTXDialog()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->artworkButton_2, SIGNAL(clicked()), this, SLOT(toggleList()));
    connect(ui->artworkButton, SIGNAL(clicked()), this, SLOT(toggleList()));
}

void NowPlayingWindow::showFMTXDialog()
{
#ifdef Q_WS_MAEMO_5
    osso_cp_plugin_execute(osso_context, "/usr/lib/hildon-control-panel/libcpfmtx.so", NULL, TRUE);
#endif
}

void NowPlayingWindow::onMetadataChanged(int songNumber, int totalNumberOfSongs, QString songName, QString albumName, QString artistName)
{
    ui->songNumberLabel->setText(QString::number(songNumber) + "/" + QString::number(totalNumberOfSongs));
    ui->songTitleLabel->setText(songName);
    ui->albumNameLabel->setText(albumName);
    ui->artistLabel->setText(artistName);
    ui->songNumberLabel_2->setText(ui->songNumberLabel->text());
    ui->songTitleLabel_2->setText(ui->songTitleLabel->text());
    ui->albumNameLabel_2->setText(ui->albumNameLabel->text());
    ui->artistLabel_2->setText(ui->artistLabel->text());
    if(!ui->songPlaylist->isHidden() || !ui->songPlaylist_2->isHidden()) {
        ui->songPlaylist->hide();
        ui->songPlaylist_2->hide();
        ui->scrollArea->show();
        ui->scrollArea_2->show();
    }
}

void NowPlayingWindow::orientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()) {
        // Landscape mode
        qDebug() << "NowPlayingWindow: Orientation changed: Landscape.";
        ui->portraitWidget->hide();
        ui->landscapeWidget->show();
    } else {
        // Portrait mode
        qDebug() << "NowPlayingWindow: Orientation changed: Portrait.";
        ui->landscapeWidget->hide();
        if(ui->scrollArea_2->isHidden())
            ui->scrollArea_2->show();
        if(!ui->songPlaylist_2->isHidden())
            ui->songPlaylist_2->hide();
        ui->portraitWidget->show();
    }
}

void NowPlayingWindow::listSongs()
{
     QDirIterator directory_walker("/home/user/MyDocs/.sounds", QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);

    while(directory_walker.hasNext())
    {
          directory_walker.next();

         if(directory_walker.fileInfo().completeSuffix() == "mp3")
                   ui->songPlaylist->addItem(directory_walker.fileName());
                   ui->songPlaylist_2->addItem(directory_walker.fileName());
    }
}

void NowPlayingWindow::toggleList()
{
    if(ui->songPlaylist->isHidden() || ui->songPlaylist_2->isHidden()) {
        ui->songPlaylist->show();
        ui->songPlaylist_2->show();
        ui->scrollArea->hide();
        ui->scrollArea_2->hide();
    } else {
        ui->songPlaylist->hide();
        ui->songPlaylist_2->hide();
        ui->scrollArea->show();
        ui->scrollArea_2->show();
    }
}
