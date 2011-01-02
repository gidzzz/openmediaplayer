#include "nowplayingwindow.h"

NowPlayingWindow::NowPlayingWindow(QWidget *parent, MafwRendererAdapter* mra) :
    QMainWindow(parent),
    ui(new Ui::NowPlayingWindow),
#ifdef Q_WS_MAEMO_5
    fmtxDialog(new FMTXDialog(this)),
    mafwrenderer(mra),
#endif
    albumArtScene(new QGraphicsScene(this))
{
    ui->setupUi(this);
    albumArtScene = new QGraphicsScene(ui->view);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    ui->volumeSlider->hide();
    ui->currentPositionLabel->setText("0:00");
    ui->trackLengthLabel->setText("0:00");
    this->setButtonIcons();
    ui->buttonsWidget->setLayout(ui->buttonsLayout);
    ui->songPlaylist->hide();
    QMainWindow::setCentralWidget(ui->verticalWidget);
    volumeTimer = new QTimer(this);
    volumeTimer->setInterval(3000);
    this->connectSignals();
}

NowPlayingWindow::~NowPlayingWindow()
{
    delete ui;
}

void NowPlayingWindow::setAlbumImage(QString image)
{
    ui->view->setScene(albumArtScene);
    albumArtScene->setBackgroundBrush(QBrush(Qt::transparent));
    m = new mirror();
    albumArtScene->addItem(m);
    QPixmap albumArt(image);
    albumArt = albumArt.scaled(QSize(300, 300));
    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(albumArt);
    albumArtScene->addItem(item);
    m->setItem(item);
    QTransform t;
    t = t.rotate(-25, Qt::YAxis);
    ui->view->setTransform(t);
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
    //ui->artworkButton->setIcon(QIcon(albumImage));
    this->setAlbumImage(albumImage);
    ui->prevButton->setIcon(QIcon(prevButtonIcon));
    ui->playButton->setIcon(QIcon(playButtonIcon));
    ui->nextButton->setIcon(QIcon(nextButtonIcon));
    ui->shuffleButton->setIcon(QIcon(shuffleButtonIcon));
    ui->repeatButton->setIcon(QIcon(repeatButtonIcon));
    ui->volumeButton->setIcon(QIcon(volumeButtonIcon));
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
  if(name == "renderer-art-uri")
  {
      this->setAlbumImage(value.toString());
  }
}

void NowPlayingWindow::stateChanged(int state)
{
  if(state == Paused)
  {
    ui->playButton->setIcon(QIcon(playButtonIcon));
    connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(pause()));
    connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(play()));
  }
  else
  {
    ui->playButton->setIcon(QIcon(pauseButtonIcon));
    disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
    connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(pause()));
  }
}

void NowPlayingWindow::connectSignals()
{
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(toggleVolumeSlider()));
    connect(ui->actionFM_Transmitter, SIGNAL(triggered()), this, SLOT(showFMTXDialog()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(volumeWatcher()));
    connect(volumeTimer, SIGNAL(timeout()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeSlider, SIGNAL(sliderPressed()), volumeTimer, SLOT(stop()));
    connect(ui->volumeSlider, SIGNAL(sliderReleased()), volumeTimer, SLOT(start()));
    connect(ui->view, SIGNAL(clicked()), this, SLOT(toggleList()));
#ifdef Q_WS_MAEMO_5
    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
    connect(mafwrenderer, SIGNAL(metadataChanged(QString, QVariant)), this, SLOT(metadataChanged(QString, QVariant)));
    connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(play()));
    connect(ui->nextButton, SIGNAL(clicked()), mafwrenderer, SLOT(next()));
    connect(ui->prevButton, SIGNAL(clicked()), mafwrenderer, SLOT(previous()));
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
    if(!ui->songPlaylist->isHidden()) {
        ui->songPlaylist->hide();
        ui->scrollArea->show();
    }
}

void NowPlayingWindow::orientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()) {
        // Landscape mode
        qDebug() << "NowPlayingWindow: Orientation changed: Landscape.";
        ui->horizontalLayout_3->setDirection(QBoxLayout::LeftToRight);
        ui->buttonsLayout->addItem(ui->horizontalSpacer_10);
        ui->volumeButton->show();
        ui->layoutWidget->setGeometry(QRect(0, 0, 372, 351));
    } else {
        // Portrait mode
        qDebug() << "NowPlayingWindow: Orientation changed: Portrait.";
        ui->horizontalLayout_3->setDirection(QBoxLayout::TopToBottom);
        ui->buttonsLayout->removeItem(ui->horizontalSpacer_10);
        ui->volumeButton->hide();
        ui->layoutWidget->setGeometry(QRect(ui->layoutWidget->rect().left(),
                                            ui->layoutWidget->rect().top(),
                                            ui->layoutWidget->width(),
                                            320));
    }
}

void NowPlayingWindow::listSongs(QString fileName)
{
    ui->songPlaylist->addItem(fileName);
}

void NowPlayingWindow::toggleList()
{
    if(ui->songPlaylist->isHidden()) {
        /* Hide scrollArea first, then show playlist otherwise
           the other labels will move a bit in portrait mode   */
        ui->scrollArea->hide();
        ui->songPlaylist->show();
    } else {
        ui->songPlaylist->hide();
        ui->scrollArea->show();
    }
}

void NowPlayingWindow::volumeWatcher()
{
    if(!ui->volumeSlider->isHidden())
        volumeTimer->start();
}
