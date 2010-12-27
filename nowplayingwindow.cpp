#include "nowplayingwindow.h"

NowPlayingWindow::NowPlayingWindow(QWidget *parent, MafwRendererAdapter* mra) :
    QMainWindow(parent),
    ui(new Ui::NowPlayingWindow),
    fmtxDialog(new FMTXDialog(this)),
    mafwrenderer(mra)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    ui->volumeSlider->hide();
    ui->currentPositionLabel->setText("0:00");
    ui->trackLengthLabel->setText("0:00");
    this->setButtonIcons();
    ui->buttonsWidget->setLayout(ui->horizontalLayout_9);
    ui->songPlaylist->hide();
    ui->songPlaylist_2->hide();
    QMainWindow::setCentralWidget(ui->horizontalWidget);
    ui->landscapeWidget->setLayout(ui->verticalLayout);
    ui->portraitWidget->setLayout(ui->verticalLayout_3);
    ui->portraitWidget->hide();
    volumeTimer = new QTimer(this);
    volumeTimer->setInterval(3000);
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
        ui->buttonsWidget->hide();
        ui->volumeSlider->show();
    } else {
        ui->buttonsWidget->show();
        ui->volumeSlider->hide();
        if(volumeTimer->isActive())
            volumeTimer->stop();
    }
}

#ifdef Q_WS_MAEMO_5
void NowPlayingWindow::onVolumeChanged(const QDBusMessage &msg)
{
    /*dbus-send --print-reply --type=method_call --dest=com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer \
                 /com/nokia/mafw/renderer/gstrenderer com.nokia.mafw.extension.get_extension_property string:volume*/
    if (msg.arguments()[0].toString() == "volume") {
        int volumeLevel = qdbus_cast<QVariant>(msg.arguments()[1]).toInt();
#ifdef DEBUG
        qDebug() << QString::number(volumeLevel);
#endif
        ui->volumeSlider->setValue(volumeLevel);
    }
}
#endif

void NowPlayingWindow::setButtonIcons()
{
    ui->artworkButton->setIcon(QIcon(albumImage));
    ui->prevButton->setIcon(QIcon(prevButtonIcon));
    ui->playButton->setIcon(QIcon(playButtonIcon));
    ui->nextButton->setIcon(QIcon(nextButtonIcon));
    ui->shuffleButton->setIcon(QIcon(shuffleButtonIcon));
    ui->repeatButton->setIcon(QIcon(repeatButtonIcon));
    ui->volumeButton->setIcon(QIcon(volumeButtonIcon));
    ui->prevButton_2->setIcon(ui->prevButton->icon());
    ui->playButton_2->setIcon(ui->playButton->icon());
    ui->nextButton_2->setIcon(ui->nextButton->icon());
    ui->shuffleButton_2->setIcon(ui->shuffleButton->icon());
    ui->repeatButton_2->setIcon(ui->repeatButton->icon());
    ui->artworkButton_2->setIcon(ui->artworkButton->icon());
}

void NowPlayingWindow::metadataChanged(QString name, QVariant value)
{
  if(name == "title" /*MAFW_METADATA_KEY_TITLE*/)
  {
    ui->songTitleLabel->setText(value.toString());
  }
  if(name == "artist" /*MAFW_METADATA_KEY_ARTIST*/)
  {
    ui->artistLabel->setText(value.toString());
  }
  if(name == "album" /*MAFW_METADATA_KEY_ALBUM*/)
  {
    ui->albumNameLabel->setText(value.toString());
  }
  this->updatePortraitWidgets();
}

void NowPlayingWindow::stateChanged(int state)
{
  if(state == Paused)
  {
    ui->playButton->setIcon(QIcon(playButtonIcon));
    ui->playButton_2->setIcon(ui->playButton->icon());
    connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(pause()));
    connect(ui->playButton_2, SIGNAL(clicked()), mafwrenderer, SLOT(pause()));
    connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(play()));
    connect(ui->playButton_2, SIGNAL(clicked()), mafwrenderer, SLOT(play()));
  }
  else
  {
    ui->playButton->setIcon(QIcon(pauseButtonIcon));
    ui->playButton_2->setIcon(ui->playButton->icon());
    disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
    disconnect(ui->playButton_2, SIGNAL(clicked()), 0, 0);
    connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(pause()));
    connect(ui->playButton_2, SIGNAL(clicked()), mafwrenderer, SLOT(pause()));
  }
}

void NowPlayingWindow::connectSignals()
{
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(toggleVolumeSlider()));
    connect(ui->actionFM_Transmitter, SIGNAL(triggered()), this, SLOT(showFMTXDialog()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->artworkButton_2, SIGNAL(clicked()), this, SLOT(toggleList()));
    connect(ui->artworkButton, SIGNAL(clicked()), this, SLOT(toggleList()));
    connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(play()));
    connect(ui->nextButton, SIGNAL(clicked()), mafwrenderer, SLOT(next()));
    connect(ui->prevButton, SIGNAL(clicked()), mafwrenderer, SLOT(previous()));
    connect(ui->playButton_2, SIGNAL(clicked()), mafwrenderer, SLOT(play()));
    connect(ui->nextButton_2, SIGNAL(clicked()), mafwrenderer, SLOT(next()));
    connect(ui->prevButton_2, SIGNAL(clicked()), mafwrenderer, SLOT(previous()));
    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
    connect(mafwrenderer, SIGNAL(metadataChanged(QString, QVariant)), this, SLOT(metadataChanged(QString, QVariant)));
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(volumeWatcher()));
    connect(volumeTimer, SIGNAL(timeout()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeSlider, SIGNAL(sliderPressed()), volumeTimer, SLOT(stop()));
    connect(ui->volumeSlider, SIGNAL(sliderReleased()), volumeTimer, SLOT(start()));
#ifdef Q_WS_MAEMO_5
    QDBusConnection::sessionBus().connect("com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer",
                                          "/com/nokia/mafw/renderer/gstrenderer",
                                          "com.nokia.mafw.extension",
                                          "property_changed",
                                          this, SLOT(onVolumeChanged(const QDBusMessage &)));
#endif
}

void NowPlayingWindow::showFMTXDialog()
{
#ifdef Q_WS_MAEMO_5
    fmtxDialog->show();
//    osso_context = osso_initialize("qt-mediaplayer", "0.1", TRUE, NULL);
//    osso_cp_plugin_execute(osso_context, "libcpfmtx.so", this, TRUE);
#endif
}

void NowPlayingWindow::onMetadataChanged(int songNumber, int totalNumberOfSongs, QString songName, QString albumName, QString artistName)
{
    ui->songNumberLabel->setText(QString::number(songNumber) + "/" + QString::number(totalNumberOfSongs) + tr(" songs"));
    ui->songTitleLabel->setText(songName);
    ui->albumNameLabel->setText(albumName);
    ui->artistLabel->setText(artistName);
    ui->songPlaylist->setCurrentRow(songNumber-1);
    if(!ui->songPlaylist->isHidden() || !ui->songPlaylist_2->isHidden()) {
        ui->songPlaylist->hide();
        ui->songPlaylist_2->hide();
        ui->scrollArea->show();
        ui->scrollArea_2->show();
    }
}

void NowPlayingWindow::updatePortraitWidgets()
{
    ui->songNumberLabel_2->setText(ui->songNumberLabel->text());
    ui->songTitleLabel_2->setText(ui->songTitleLabel->text());
    ui->albumNameLabel_2->setText(ui->albumNameLabel->text());
    ui->artistLabel_2->setText(ui->artistLabel->text());
    ui->songPlaylist_2->setCurrentRow(ui->songPlaylist->currentRow());
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
                   ui->songPlaylist->addItem(directory_walker.fileName());
                   ui->songPlaylist_2->addItem(directory_walker.fileName());
    }
}

void NowPlayingWindow::toggleList()
{
    if(ui->songPlaylist->isHidden() || ui->songPlaylist_2->isHidden()) {
        /* Hide scrollArea first, then show playlist otherwise
           the other labels will move a bit in portrait mode   */
        ui->scrollArea->hide();
        ui->scrollArea_2->hide();
        ui->songPlaylist->show();
        ui->songPlaylist_2->show();
    } else {
        ui->songPlaylist->hide();
        ui->songPlaylist_2->hide();
        ui->scrollArea->show();
        ui->scrollArea_2->show();
    }
}

void NowPlayingWindow::volumeWatcher()
{
    if(!ui->volumeSlider->isHidden())
        volumeTimer->start();
}
