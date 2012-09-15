/**************************************************************************
    This file is part of Open MediaPlayer
    Copyright (C) 2010-2011 Mohammad Abu-Garbeyyeh

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "videonowplayingwindow.h"

VideoNowPlayingWindow::VideoNowPlayingWindow(QWidget *parent, MafwAdapterFactory *factory, bool overlay) :
    QMainWindow(parent),
    ui(new Ui::VideoNowPlayingWindow)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwSource(factory->getTempSource())
#endif
{
    /* Make Qt do the work of keeping the overlay the magic color  */
    QWidget::setBackgroundRole(QPalette::Window);
    QWidget::setAutoFillBackground(true);
    QPalette overlayPalette = QWidget::palette();
    overlayPalette.setColor(QPalette::Window, colorKey());
    QWidget::setPalette(overlayPalette);

    ui->setupUi(this);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_DeleteOnClose);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);

    /*quint32 disable = {0};
    Atom winPortraitModeSupportAtom = XInternAtom(QX11Info::display(), "_HILDON_PORTRAIT_MODE_SUPPORT", false);
    XChangeProperty(QX11Info::display(), winId(), winPortraitModeSupportAtom, XA_CARDINAL, 32, PropModeReplace, (uchar*) &disable, 1);*/
    //http://www.gossamer-threads.com/lists/maemo/developers/54239

    Rotator *rotator = Rotator::acquire();
    savedPolicy = rotator->policy();
    rotator->setPolicy(Rotator::Landscape);
    orientationChanged(rotator->width(), rotator->height());
#endif

    volumeTimer = new QTimer(this);
    volumeTimer->setInterval(3000);

    positionTimer = new QTimer(this);
    positionTimer->setInterval(1000);

    lazySliders = QSettings().value("main/lazySliders").toBool();
    reverseTime = QSettings().value("main/reverseTime").toBool();

    this->overlayRequestedByUser = overlay;
    this->saveStateOnClose = true;
    this->overlayVisible = true;
    this->gotInitialState = false;
    this->buttonWasDown = false;
#ifdef MAFW
    this->errorOccured = false;
#endif

    this->setIcons();
    this->connectSignals();

    ui->currentPositionLabel->installEventFilter(this);

    showOverlay(overlay);

#ifdef MAFW
    mafwrenderer->setColorKey(199939);
    mafwrenderer->getVolume();
    ui->toolbarOverlay->setStyleSheet(ui->controlOverlay->styleSheet());
#endif

    QApplication::syncX();
    mafwrenderer->getStatus();
    mafwrenderer->setWindowXid(ui->videoWidget->winId());
}

VideoNowPlayingWindow::~VideoNowPlayingWindow()
{
#ifdef MAFW
    if (saveStateOnClose) {
        if (mafwState != Paused && mafwState != Stopped)
            mafwrenderer->pause();

        if (mafwSource) {
            GHashTable* metadata = mafw_metadata_new();
            if (currentPosition == 0)
                mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI, "");
            mafw_metadata_add_int(metadata, MAFW_METADATA_KEY_PAUSED_POSITION, currentPosition);
            mafwSource->setMetadata(currentObjectId.toUtf8(), metadata);
            mafw_metadata_release(metadata);
        }

        mafwrenderer->stop();
    }
#endif

    Rotator::acquire()->setPolicy(savedPolicy);

    delete ui;
}

bool VideoNowPlayingWindow::eventFilter(QObject*, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease)
        return true;

    if (event->type() == QEvent::MouseButtonPress) {
        reverseTime = !reverseTime;
        QSettings().setValue("main/reverseTime", reverseTime);
        ui->currentPositionLabel->setText(time_mmss(reverseTime ? ui->progressBar->value()-videoLength :
                                                                  ui->progressBar->value()));
        return true;
    }
    return false;
}

void VideoNowPlayingWindow::setIcons()
{
    ui->wmCloseButton->setIcon(QIcon(wmCloseIcon));
    ui->prevButton->setIcon(QIcon(prevButtonIcon));
    ui->playButton->setIcon(QIcon(playButtonIcon));
    ui->nextButton->setIcon(QIcon(nextButtonIcon));
    ui->bookmarkButton->setIcon(QIcon::fromTheme(bookmarkButtonIcon));
    ui->deleteButton->setIcon(QIcon::fromTheme(deleteButtonIcon));
    ui->shareButton->setIcon(QIcon::fromTheme(shareButtonIcon));
    ui->volumeButton->setIcon(QIcon(volumeButtonIcon));
}

void VideoNowPlayingWindow::connectSignals()
{
    QShortcut *shortcut;

    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Space), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), this, SLOT(toggleOverlay()));

    connect(ui->prevButton, SIGNAL(clicked()), this, SLOT(onPrevButtonClicked()));
    connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(onNextButtonClicked()));

    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(volumeWatcher()));
    connect(ui->volumeSlider, SIGNAL(sliderPressed()), this, SLOT(onVolumeSliderPressed()));
    connect(ui->volumeSlider, SIGNAL(sliderReleased()), this, SLOT(onVolumeSliderReleased()));
    connect(volumeTimer, SIGNAL(timeout()), this, SLOT(toggleVolumeSlider()));

    connect(ui->bookmarkButton, SIGNAL(clicked()), this, SLOT(onBookmarkClicked()));
    connect(ui->shareButton, SIGNAL(clicked()), this, SLOT(onShareClicked()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(onDeleteClicked()));

#ifdef MAFW
    shortcut = new QShortcut(QKeySequence(Qt::Key_Space), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), this, SLOT(togglePlayback()));
    connect(new QShortcut(QKeySequence(Qt::Key_Left), this), SIGNAL(activated()), this, SLOT(slowRev()));
    connect(new QShortcut(QKeySequence(Qt::Key_Right), this), SIGNAL(activated()), this, SLOT(slowFwd()));
    connect(new QShortcut(QKeySequence(Qt::Key_Up), this), SIGNAL(activated()), this, SLOT(fastFwd()));
    connect(new QShortcut(QKeySequence(Qt::Key_Down), this), SIGNAL(activated()), this, SLOT(fastRev()));

    connect(mafwrenderer, SIGNAL(bufferingInfo(float)), this, SLOT(onBufferingInfo(float)));
    connect(mafwrenderer, SIGNAL(mediaChanged(int,char*)), this, SLOT(onMediaChanged(int,char*)));
    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    connect(mafwrenderer, SIGNAL(signalGetPosition(int,QString)), this, SLOT(onPositionChanged(int,QString)));
    connect(mafwrenderer, SIGNAL(mediaIsSeekable(bool)), ui->progressBar, SLOT(setEnabled(bool)));
    connect(mafwrenderer, SIGNAL(signalGetVolume(int)), ui->volumeSlider, SLOT(setValue(int)));
    connect(mafwrenderer, SIGNAL(metadataChanged(QString,QVariant)),
            this, SLOT(onMetadataChanged(QString,QVariant)));
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));
    connect(mafwrenderer, SIGNAL(signalGetCurrentMetadata(GHashTable*,QString,QString)),
            this, SLOT(onRendererMetadataRequested(GHashTable*,QString,QString)));

    connect(mafwSource, SIGNAL(signalMetadataResult(QString,GHashTable*,QString)),
            this, SLOT(onSourceMetadataRequested(QString, GHashTable*, QString)));

    connect(positionTimer, SIGNAL(timeout()), mafwrenderer, SLOT(getPosition()));
    connect(ui->volumeSlider, SIGNAL(sliderMoved(int)), mafwrenderer, SLOT(setVolume(int)));
    connect(ui->progressBar, SIGNAL(sliderPressed()), this, SLOT(onSliderPressed()));
    connect(ui->progressBar, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));
    connect(ui->progressBar, SIGNAL(sliderMoved(int)), this, SLOT(onSliderMoved(int)));

    QDBusConnection::sessionBus().connect("com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer",
                                          "/com/nokia/mafw/renderer/gstrenderer",
                                          "com.nokia.mafw.extension",
                                          "property_changed",
                                          this, SLOT(onPropertyChanged(const QDBusMessage &)));

    QDBusConnection::sessionBus().connect("",
                                          "/com/nokia/mafw/renderer/gstrenderer",
                                          "com.nokia.mafw.extension",
                                          "error",
                                          this, SLOT(onErrorOccured(const QDBusMessage &)));

#endif
}

#ifdef MAFW
void VideoNowPlayingWindow::onMetadataChanged(QString name, QVariant value)
{
    qDebug() << "Metadata changed:" << name << "=" << value;

    if (name == MAFW_METADATA_KEY_RES_X)
        videoWidth = value.toInt();
    else if (name == MAFW_METADATA_KEY_RES_Y)
        videoHeight = value.toInt();

    if (videoWidth && videoHeight && QSettings().value("Videos/fitToScreen").toBool())
        setAspectRatio((float) videoWidth / videoHeight);

    mafwrenderer->getCurrentMetadata();

    // duration sometimes is misreported for UPnP, so don't set it from here unnecessarily
    if (videoLength == Duration::Unknown && name == "duration") {
        videoLength = value.toInt();
        ui->videoLengthLabel->setText(time_mmss(videoLength));
        ui->progressBar->setRange(0, videoLength);
    }
}
#endif

#ifdef MAFW
void VideoNowPlayingWindow::onRendererMetadataRequested(GHashTable *metadata, QString, QString error)
{
    if (metadata != NULL
#ifdef MAFW_WORKAROUNDS
    // Looks like the renderer cannot tell us the video codec in RTSP streams.
    // Being able to play something without knowing the codec smells fishy,
    // so I take it for a bug in MAFW and I'm putting this as a workaround.
    && !currentObjectId.startsWith("urisource::rtsp://")
#endif
    && mafw_metadata_first(metadata, MAFW_METADATA_KEY_AUDIO_CODEC)
    && !mafw_metadata_first(metadata, MAFW_METADATA_KEY_VIDEO_CODEC)) {
        qDebug() << "Video codec info unavailable, switching to radio mode";

        MafwPlaylistManagerAdapter *playlistManager = mafwFactory->getPlaylistAdapter()->mafw_playlist_manager;
        playlistManager->deletePlaylist("FmpRadioPlaylist");
        mafw_playlist_set_name(MAFW_PLAYLIST(playlistManager->createPlaylist("FmpVideoPlaylist")), "FmpRadioPlaylist");

        RadioNowPlayingWindow *window = new RadioNowPlayingWindow(this->parentWidget(), mafwFactory);
        saveStateOnClose = false;
        delete this;
        window->show();
    }

    if (!error.isEmpty())
        qDebug() << error;
}
#endif

#ifdef MAFW
void VideoNowPlayingWindow::onGetStatus(MafwPlaylist*, uint index, MafwPlayState state, const char* object_id, QString error)
{
    disconnect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
               this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));

    onStateChanged(state);
    onMediaChanged(index, const_cast<char*>(object_id));

    if (!error.isEmpty())
        qDebug() << error;
}
#endif

#ifdef MAFW
void VideoNowPlayingWindow::onMediaChanged(int, char* objectId)
{
    currentObjectId = QString::fromUtf8(objectId);
    mafwSource->setSource(mafwFactory->getTempSource()->getSourceByUUID(currentObjectId.left(currentObjectId.indexOf("::"))));
    mafwSource->getMetadata(currentObjectId.toUtf8(), MAFW_SOURCE_LIST(MAFW_METADATA_KEY_URI,
                                                                       MAFW_METADATA_KEY_DURATION,
                                                                       MAFW_METADATA_KEY_PAUSED_POSITION));

    if (currentObjectId.startsWith("localtagfs::")) { // local storage
        ui->bookmarkButton->hide();
        ui->shareButton->show();
        ui->deleteButton->show();
        ui->toolbarLayout->setContentsMargins(0,0,0,0);
    } else { // remote sources
        ui->shareButton->hide();
        ui->deleteButton->hide();
        ui->bookmarkButton->show();
        ui->toolbarLayout->setContentsMargins(20,0,0,0);
    }

    videoLength = Duration::Unknown;

    videoWidth = videoHeight = 0;
    setAspectRatio(800.0 / 480.0);

    onBufferingInfo(1.0);
}
#endif

void VideoNowPlayingWindow::onPrevButtonClicked()
{
#ifdef MAFW
    if (ui->prevButton->isDown()) {
        buttonWasDown = true;
        mafwrenderer->setPosition(SeekRelative, -10);
        mafwrenderer->getPosition();
    } else {
        if (!buttonWasDown) {
            if (this->currentPosition > 3) {
                mafwrenderer->setPosition(SeekAbsolute, 0);
                mafwrenderer->getPosition();
            } else {
                gotInitialState = false;
                mafwrenderer->previous();
            }
        }
        buttonWasDown = false;
    }
#endif
}

void VideoNowPlayingWindow::onNextButtonClicked()
{
#ifdef MAFW
    if (ui->nextButton->isDown()) {
        buttonWasDown = true;
        mafwrenderer->setPosition(SeekRelative, 10);
        mafwrenderer->getPosition();
    } else {
        if (!buttonWasDown) {
            gotInitialState = false;
            mafwrenderer->next();
        }
        buttonWasDown = false;
    }
#endif
}

void VideoNowPlayingWindow::onBookmarkClicked()
{
#ifdef MAFW
    mafwrenderer->pause();
    BookmarkDialog bookmarkDialog(this, mafwFactory, Media::Video, uri);
    bookmarkDialog.exec();
#endif
}

void VideoNowPlayingWindow::onDeleteClicked()
{
#ifdef MAFW
    mafwrenderer->pause();

    if (ConfirmDialog(ConfirmDialog::DeleteVideo, this).exec() == QMessageBox::Yes) {
        mafwSource->destroyObject(currentObjectId.toUtf8());
        this->close();
    }
#endif
}

void VideoNowPlayingWindow::onShareClicked()
{
#ifdef MAFW
    mafwrenderer->pause();
    mafwSource->getUri(currentObjectId.toUtf8());
    connect(mafwSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void VideoNowPlayingWindow::onShareUriReceived(QString objectId, QString uri)
{
    disconnect(mafwSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));

    if (objectId != currentObjectId) return;

    QStringList files;
#ifdef DEBUG
    qDebug() << "Sending file:" << uri;
#endif
    files.append(uri);
#ifdef Q_WS_MAEMO_5
    ShareDialog(this, files).exec();
#endif
}
#endif

void VideoNowPlayingWindow::onVolumeSliderPressed()
{
    volumeTimer->stop();
#ifdef MAFW
    mafwrenderer->setVolume(ui->volumeSlider->value());
#endif
}

void VideoNowPlayingWindow::onVolumeSliderReleased()
{
    volumeTimer->start();
#ifdef MAFW
    mafwrenderer->setVolume(ui->volumeSlider->value());
#endif
}

void VideoNowPlayingWindow::toggleVolumeSlider()
{
    if (ui->volumeSlider->isHidden()) {
        ui->buttonWidget->hide();
        ui->volumeSlider->show();
    } else {
        ui->volumeSlider->hide();
        ui->buttonWidget->show();
        if (volumeTimer->isActive())
            volumeTimer->stop();
    }
}

#ifdef MAFW
void VideoNowPlayingWindow::onPropertyChanged(const QDBusMessage &msg)
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

    else if (msg.arguments()[0].toString() == "colorkey") {
        colorkey = qdbus_cast<QVariant>(msg.arguments()[1]).toInt();
    }
}
#endif

void VideoNowPlayingWindow::volumeWatcher()
{
    if (!ui->volumeSlider->isHidden())
        volumeTimer->start();
}

#ifdef MAFW
void VideoNowPlayingWindow::onStateChanged(int state)
{
    if (state != Stopped && state != Transitioning) gotInitialState = true;
    mafwState = state;

    if (state == Transitioning) {
        if (gotInitialState && !QSettings().value("Videos/continuousPlayback", false).toBool()) {
            mafwrenderer->stop();
            mafwrenderer->previous();
        }

        ui->progressBar->setEnabled(false);
        ui->progressBar->setValue(0);
        ui->currentPositionLabel->setText("00:00");

        if (ui->bufferBar->maximum() == 0
        && !currentObjectId.startsWith("localtagfs::")
        && !currentObjectId.startsWith("urisource::file://")) {
            showOverlay(true);
            ui->positionWidget->hide();
            ui->bufferBar->show();
        }
    } else {
        if (ui->bufferBar->maximum() == 0) {
            showOverlay(overlayRequestedByUser);
            ui->bufferBar->hide();
            ui->positionWidget->show();
        }

        if (state == Paused) {
            ui->playButton->setIcon(QIcon(playButtonIcon));
            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
            connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(resume()));
#ifdef Q_WS_MAEMO_5
            this->setDNDAtom(false);
#endif
            mafwrenderer->getPosition();
            this->pausedPosition = this->currentPosition;
            mafwrenderer->getPosition();
            if (positionTimer->isActive())
                positionTimer->stop();
        }
        else if (state == Playing) {
            ui->playButton->setIcon(QIcon(pauseButtonIcon));
            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
            connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(pause()));
            if (pausedPosition != -1 && pausedPosition != 0)
                mafwrenderer->setPosition(SeekAbsolute, pausedPosition);
            mafwrenderer->getPosition();
#ifdef Q_WS_MAEMO_5
            this->setDNDAtom(true);
#endif
            if (!positionTimer->isActive())
                positionTimer->start();
        }
        else if (state == Stopped) {
            currentPosition = 0;
            ui->playButton->setIcon(QIcon(playButtonIcon));
            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
            connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(play()));
#ifdef Q_WS_MAEMO_5
            this->setDNDAtom(false);
#endif
            if (positionTimer->isActive())
                positionTimer->stop();
            if (gotInitialState && !errorOccured) {
                this->close();
                delete this; // why is it not deleted automatically, despite WA_DeleteOnClose?
            }
        }
    }
}
#endif

void VideoNowPlayingWindow::orientationChanged(int w, int h)
{
    ui->controlOverlay->setGeometry((w - ui->controlOverlay->width())/2, (h - ui->controlOverlay->height())/2,
                                    ui->controlOverlay->width(), ui->controlOverlay->height());
    ui->toolbarOverlay->setGeometry(0, h-ui->toolbarOverlay->height(),
                                    w, ui->toolbarOverlay->height());
    ui->wmCloseButton->setGeometry(w-ui->wmCloseButton->width(), 0,
                                   ui->wmCloseButton->width(), ui->wmCloseButton->height());

    ui->controlLayout->setDirection(QBoxLayout::LeftToRight);
    ui->volumeButton->show();
    ui->toolbarOverlay->show();
}

void VideoNowPlayingWindow::setAspectRatio(float ratio)
{
    int w, h;

    if (ratio > 800.0/480.0) {
        w = ratio * 480;
        h = 480;
    } else {
        w = 800;
        h = 800 / ratio;
    }

    if (w != ui->videoWidget->width() || h != ui->videoWidget->height()) {
        qDebug() << "Aspect ratio chaged to" << ratio;
        ui->videoWidget->setGeometry((800-w)/2, (480-h)/2, w, h);
    }
}

#ifdef Q_WS_MAEMO_5
void VideoNowPlayingWindow::setDNDAtom(bool dnd)
{
    quint32 enable = dnd ? 1 : 0;
    Atom winDNDAtom = XInternAtom(QX11Info::display(), "_HILDON_DO_NOT_DISTURB", false);
    XChangeProperty(QX11Info::display(), winId(), winDNDAtom, XA_INTEGER, 32, PropModeReplace, (uchar*) &enable, 1);
}
#endif

#ifdef MAFW
void VideoNowPlayingWindow::onSourceMetadataRequested(QString objectId, GHashTable *metadata, QString error)
{
    if (objectId != currentObjectId) return;

    if (metadata != NULL) {
        GValue *v;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_URI);
        uri = v ? g_value_get_string (v) : QString();

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_DURATION);
        if (v) videoLength = g_value_get_int (v);

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_PAUSED_POSITION);
        pausedPosition = v ? g_value_get_int (v) : -1;

        if (pausedPosition != -1)
            qDebug() << "paused position:" << pausedPosition;

        ui->videoLengthLabel->setText(time_mmss(videoLength));
        ui->progressBar->setRange(0, videoLength);

    }

    if (!error.isEmpty())
        qDebug() << error;
}

void VideoNowPlayingWindow::slowFwd()
{
    mafwrenderer->setPosition(SeekRelative, 10);
    if (mafwState == Paused)
        mafwrenderer->getPosition();
}

void VideoNowPlayingWindow::slowRev()
{
    mafwrenderer->setPosition(SeekRelative, -10);
    if (mafwState == Paused)
        mafwrenderer->getPosition();
}

void VideoNowPlayingWindow::fastFwd()
{
    mafwrenderer->setPosition(SeekRelative, 60);
    if (mafwState == Paused)
        mafwrenderer->getPosition();
}

void VideoNowPlayingWindow::fastRev()
{
    mafwrenderer->setPosition(SeekRelative, -60);
    if (mafwState == Paused)
        mafwrenderer->getPosition();
}
#endif

void VideoNowPlayingWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QBrush brush;
    brush.setColor(QColor(3, 13, 3));
    painter.setBrush(brush);
    QPalette palette = this->palette();
    palette.setColor(this->backgroundRole(), QColor(3, 13, 3));
}

void VideoNowPlayingWindow::mouseReleaseEvent(QMouseEvent *)
{
    toggleOverlay();
}

void VideoNowPlayingWindow::toggleOverlay()
{
    overlayRequestedByUser = !overlayVisible;
    showOverlay(overlayRequestedByUser);
}

void VideoNowPlayingWindow::togglePlayback()
{
    if (this->mafwState == Playing)
        mafwrenderer->pause();
    else if (this->mafwState == Paused)
        mafwrenderer->resume();
    else if (this->mafwState == Stopped)
        mafwrenderer->play();
}

void VideoNowPlayingWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

void VideoNowPlayingWindow::showOverlay(bool show)
{
    ui->controlOverlay->setHidden(!show);
    ui->toolbarOverlay->setHidden(!show);
    ui->wmCloseButton->setHidden(!show);

    overlayVisible = show;

#ifdef MAFW
    if (!show && this->positionTimer->isActive())
        this->positionTimer->stop();
    else if (show && !this->positionTimer->isActive() && this->mafwState != Stopped) {
        this->positionTimer->start();
        mafwrenderer->getPosition();
    }
#endif
}

void VideoNowPlayingWindow::onSliderPressed()
{
    this->onSliderMoved(ui->progressBar->value());
}

void VideoNowPlayingWindow::onSliderReleased()
{
#ifdef MAFW
    mafwrenderer->setPosition(SeekAbsolute, ui->progressBar->value());
    ui->currentPositionLabel->setText(time_mmss(reverseTime ? ui->progressBar->value()-videoLength :
                                                              ui->progressBar->value()));
#endif
}

void VideoNowPlayingWindow::onSliderMoved(int position)
{
    ui->currentPositionLabel->setText(time_mmss(reverseTime ? position-videoLength : position));
#ifdef MAFW
    if (!lazySliders)
        mafwrenderer->setPosition(SeekAbsolute, position);
#endif
}

#ifdef MAFW
void VideoNowPlayingWindow::onBufferingInfo(float status)
{
    if (status == 1.0) {
        showOverlay(overlayRequestedByUser);
        ui->bufferBar->hide();
        ui->positionWidget->show();
        ui->bufferBar->setRange(0, 0);
    } else { // status != 1.0
        int percentage = (int)(status*100);
        ui->bufferBar->setRange(0, 100);
        ui->bufferBar->setValue(percentage);
        ui->bufferBar->setFormat(tr("Buffering") + " %p%");

        if (ui->bufferBar->isHidden()) {
            ui->positionWidget->hide();
            ui->bufferBar->show();
            showOverlay(true);
        }
    }
}
#endif

#ifdef MAFW
void VideoNowPlayingWindow::onPositionChanged(int position, QString)
{
    this->currentPosition = position;
    if (this->mafwState == Paused)
         this->pausedPosition = position;
    if (!ui->progressBar->isSliderDown() && ui->progressBar->isVisible()) {
        ui->currentPositionLabel->setText(time_mmss(reverseTime ? position-videoLength : position));
        if (ui->progressBar->isEnabled())
            ui->progressBar->setValue(position);
    }
}
#endif

#ifdef MAFW
void VideoNowPlayingWindow::onErrorOccured(const QDBusMessage &msg)
{
    this->errorOccured = true;
    QString errorMsg;

    if (msg.arguments()[0] == "com.nokia.mafw.error.renderer") {
        errorMsg.append(tr("Unable to play media"));
        errorMsg.append("\n");

        if (msg.arguments()[1] == MAFW_RENDERER_ERROR_NO_MEDIA)
            errorMsg.append(tr("Media not found"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_URI_NOT_AVAILABLE)
            errorMsg.append(tr("URI not available"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_INVALID_URI)
            errorMsg.append(tr("Invalid URI"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_MEDIA_NOT_FOUND)
            errorMsg.append(tr("Unable to open media"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_STREAM_DISCONNECTED)
            errorMsg.append(tr("Playback stream no longer available"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_TYPE_NOT_AVAILABLE)
            errorMsg.append(tr("Could not determine MIME-type"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_PLAYBACK)
            errorMsg.append(tr("General error occured, unable to continue playback"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_UNABLE_TO_PERFORM)
            errorMsg.append(tr("General error occured"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_UNSUPPORTED_TYPE)
            errorMsg.append(tr("Unsupported media"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_UNSUPPORTED_RESOLUTION)
            errorMsg.append(tr("Unsupported resolution"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_UNSUPPORTED_FPS)
            errorMsg.append(tr("Unsupported framerate"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_DRM)
            errorMsg.append(tr("Media is protected by DRM"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_DEVICE_UNAVAILABLE)
            errorMsg.append(tr("System sound device is unavailable"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CORRUPTED_FILE)
            errorMsg.append(tr("Media corrupted"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_PLAYLIST_PARSING) {
            errorMsg.append(tr("Error while parsing playlist"));
            errorMsg.append(tr("Playlist may be corrupt or empty"));
        }
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CODEC_NOT_FOUND) {
            errorMsg.append(tr("Codec not found:") + "\n");
            errorMsg.append(msg.arguments()[2].toString());
        }
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_VIDEO_CODEC_NOT_FOUND) {
            errorMsg.append(tr("Video codec not found:") + "\n");
            errorMsg.append(msg.arguments()[2].toString());
        }
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_AUDIO_CODEC_NOT_FOUND) {
            errorMsg.append(tr("Audio codec not found:") + "\n");
            errorMsg.append(msg.arguments()[2].toString());
        }
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_NO_PLAYLIST)
            errorMsg.append(tr("No playlist assigned"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_INDEX_OUT_OF_BOUNDS)
            errorMsg.append(tr("Media index is not in bound with playlist items"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CANNOT_PLAY)
            errorMsg.append(tr("Unable to start playback"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CANNOT_STOP)
            errorMsg.append(tr("Unable to stop playback"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CANNOT_PAUSE)
            errorMsg.append(tr("Unable to pause playback"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CANNOT_SET_POSITION)
            errorMsg.append(tr("Unable to seek position in media"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CANNOT_GET_POSITION)
            errorMsg.append(tr("Unable to retrieve current position in media"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CANNOT_GET_STATUS)
            errorMsg.append(tr("Unable to get current playback status"));

#ifdef Q_WS_MAEMO_5
        QMaemo5InformationBox *box = new QMaemo5InformationBox(this);
        box->setAttribute(Qt::WA_DeleteOnClose);

        QWidget *widget = new QWidget(box);
        QSpacerItem *spacer = new QSpacerItem(90, 20, QSizePolicy::Fixed, QSizePolicy::Maximum);

        QLabel *errorLabel = new QLabel(box);

        QHBoxLayout *layout = new QHBoxLayout(widget);

        layout->addItem(spacer);
        layout->addWidget(errorLabel);
        layout->setSpacing(0);

        widget->setLayout(layout);

        // Bad padding in default widget, use tabbing to cover it up, sigh :/
        errorLabel->setText("\n" + errorMsg + "\n");
        errorLabel->setAlignment(Qt::AlignLeft);
        errorLabel->setWordWrap(true);

        box->setWidget(widget);
        box->setTimeout(QMaemo5InformationBox::NoTimeout);

        connect(box, SIGNAL(clicked()), this, SLOT(close()));

        box->exec();
#else
        QMessageBox::critical(this, tr("Unable to play media") + "\n", errorMsg);
#endif
    }
}
#endif

