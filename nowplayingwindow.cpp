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

NowPlayingWindow::NowPlayingWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NowPlayingWindow)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    ui->volumeSlider->hide();
    QMainWindow::setCentralWidget(ui->verticalLayoutWidget);
    this->setButtonIcons();
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(toggleVolumeSlider()));
    connect(ui->actionFM_Transmitter, SIGNAL(triggered()), this, SLOT(showFMTXDialog()));
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
