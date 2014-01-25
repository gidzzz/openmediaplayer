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
    ui->setupUi(this);
    setIcons();

    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_DeleteOnClose);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);

    // Lock the orientation to landscape, but remember the policy to restore it later
    Rotator *rotator = Rotator::acquire();
    rotatorPolicy = rotator->policy();
    rotator->setPolicy(Rotator::Landscape);
#endif

    // The timer to hide the volume slider
    volumeTimer = new QTimer(this);
    volumeTimer->setInterval(3000);

    // The timer to refresh playback progress
    positionTimer = new QTimer(this);
    positionTimer->setInterval(1000);

    // Autorepeating of playback controls
    keyRepeatTimer = new QTimer(this);
    keyRepeatTimer->setInterval(250);
    ui->prevButton->setAutoRepeatInterval(250);
    ui->nextButton->setAutoRepeatInterval(250);

    // Load saved settings
    QSettings settings;
    lazySliders = settings.value("main/lazySliders").toBool();
    reverseTime = settings.value("main/reverseTime").toBool();
    showSettings = settings.value("Videos/showSettings").toBool();
    ui->fitCheckBox->setChecked(fitToScreen = settings.value("Videos/fitToScreen").toBool());
    ui->continuousCheckBox->setChecked(settings.value("Videos/continuousPlayback").toBool());

    // Intent flags
    playWhenReady = false;
    saveStateOnClose = true;

    // State flags
    gotInitialStopState = false;
    gotInitialPlayState = false;
    gotCurrentPlayState = false;
    buttonWasDown = false;

    resumePosition = Duration::Unknown;

    overlayRequestedByUser = overlay;
    showOverlay(overlay);

    // Signals and events
    connectSignals();
    ui->currentPositionLabel->installEventFilter(this);

#ifdef MAFW
    // Set up video surface
    QApplication::syncX();
    mafwrenderer->setColorKey(colorKey().rgb() & 0xffffff);
    mafwrenderer->setWindowXid(ui->videoWidget->winId());

    // Do not jump to the next playlist item if an error occurs
    mafwrenderer->setErrorPolicy(MAFW_RENDERER_ERROR_POLICY_STOP);

    // Request some initial values
    mafwrenderer->getStatus();
    mafwrenderer->getVolume();
#endif
}

VideoNowPlayingWindow::~VideoNowPlayingWindow()
{
#ifdef MAFW
    if (saveStateOnClose) {
        // The state should be saved only if resuming is already disabled,
        // to prevent overwriting a position that is waiting to be resumed.
        if (resumePosition == Duration::Blank) {
            // Pausing the video should cause a thumbnail to be generated
            if (mafwState == Playing)
                mafwrenderer->pause();

            // Prepare a metadata table to save the state in it
            GHashTable* metadata = mafw_metadata_new();

            qDebug() << "Saving position" << currentPosition << "for" << playedObjectId;

            // Store the current position so that the playback can be resumed later
            mafw_metadata_add_int(metadata, MAFW_METADATA_KEY_PAUSED_POSITION, currentPosition);

            // If the position is at the beginning, the pause thumbnail should be reset
            if (currentPosition == 0)
                mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI, "");

            // Commit the metadata in the table
            mafwSource->setMetadata(playedObjectId.toUtf8(), metadata);
            mafw_metadata_release(metadata);
        }

        // Make sure that video playback is dead after the window is closed,
        // because otherwise ghost windows may appear, rapidly opening and closing.
        mafwrenderer->stop();
        // NOTE: If ghost windows happen despite the line above, it might be
        // necessary to wait for a confirmation that the playback is stopped and
        // only then close the window.
    }
#endif

    // Restore the default error policy
    mafwrenderer->setErrorPolicy(MAFW_RENDERER_ERROR_POLICY_CONTINUE);

    // Restore the rotation policy to the one used before opening this window
    Rotator::acquire()->setPolicy(rotatorPolicy);

    delete ui;
}

// Handle clicks from the position label to toggle between its normal and reverse mode
bool VideoNowPlayingWindow::eventFilter(QObject*, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease)
        return true;

    if (event->type() == QEvent::MouseButtonPress) {
        reverseTime = !reverseTime;
        QSettings().setValue("main/reverseTime", reverseTime);
        ui->currentPositionLabel->setText(mmss_pos(reverseTime ? ui->positionSlider->value()-videoLength :
                                                                 ui->positionSlider->value()));
        return true;
    }

    return false;
}

void VideoNowPlayingWindow::onScreenLocked(bool locked)
{
    if (locked) {
        positionTimer->stop();
    } else {
        startPositionTimer();
    }
}

void VideoNowPlayingWindow::setIcons()
{
    ui->wmEditButton->setIcon(QIcon(wmEditIcon));
    ui->wmCloseButton->setIcon(QIcon(wmCloseIcon));

    ui->prevButton->setIcon(QIcon(prevButtonIcon));
    ui->playButton->setIcon(QIcon(playButtonIcon));
    ui->nextButton->setIcon(QIcon(nextButtonIcon));

    ui->bookmarkButton->setIcon(QIcon::fromTheme(bookmarkButtonIcon));
    ui->shareButton->setIcon(QIcon::fromTheme(shareButtonIcon));
    ui->deleteButton->setIcon(QIcon::fromTheme(deleteButtonIcon));
    ui->volumeButton->setIcon(QIcon::fromTheme(volumeButtonOverlayIcon));
}

void VideoNowPlayingWindow::connectSignals()
{
    QShortcut *shortcut;

    // Controls on the settings bar
    connect(ui->wmCloseButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->wmEditButton, SIGNAL(clicked()), this, SLOT(toggleSettings()));
    connect(ui->fitCheckBox, SIGNAL(toggled(bool)), this, SLOT(setFitToScreen(bool)));
    connect(ui->continuousCheckBox, SIGNAL(toggled(bool)), this, SLOT(setContinuousPlayback(bool)));

    // Action buttons
    connect(ui->bookmarkButton, SIGNAL(clicked()), this, SLOT(onBookmarkClicked()));
    connect(ui->shareButton, SIGNAL(clicked()), this, SLOT(onShareClicked()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(onDeleteClicked()));

    // Playback buttons
    connect(ui->prevButton, SIGNAL(clicked()), this, SLOT(onPrevButtonClicked()));
    connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(onNextButtonClicked()));

    // Volume slider
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeSlider, SIGNAL(sliderPressed()), this, SLOT(onVolumeSliderPressed()));
    connect(ui->volumeSlider, SIGNAL(sliderReleased()), this, SLOT(onVolumeSliderReleased()));
    connect(ui->volumeSlider, SIGNAL(sliderMoved(int)), mafwrenderer, SLOT(setVolume(int)));
    connect(volumeTimer, SIGNAL(timeout()), this, SLOT(toggleVolumeSlider()));

    // A shortcut to toggle the overlay
    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Space), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), this, SLOT(toggleOverlay()));

    // Shortcuts to control the playback
    shortcut = new QShortcut(QKeySequence(Qt::Key_Space), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), this, SLOT(togglePlayback()));
    connect(keyRepeatTimer, SIGNAL(timeout()), this, SLOT(repeatKey()));

    // Screen locking
    connect(Maemo5DeviceEvents::acquire(), SIGNAL(screenLocked(bool)), this, SLOT(onScreenLocked(bool)));

    // Initial status
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));

    // Metadata
    MetadataWatcher *mw = MissionControl::acquire()->metadataWatcher();
    connect(mw, SIGNAL(metadataChanged(QString,QVariant)), this, SLOT(onMetadataChanged(QString,QVariant)));
    QMapIterator<QString,QVariant> i(mw->metadata()); i.toBack();
    while (i.hasPrevious()) {
        // Going backwards is necessary for media type detection to work, which
        // depends on video codec arriving before audio codec.
        i.previous();
        onMetadataChanged(i.key(), i.value());
    }

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
}

void VideoNowPlayingWindow::onMetadataChanged(QString key, QVariant value)
{
    if (key == MAFW_METADATA_KEY_AUDIO_CODEC) {
        // Try to detect whether we have a video or an audio-only stream. If not
        // sure, take the safe approach by keeping the video window open.
        // NOTE: The implemented solution assumes that video codec always arrives
        // before audio codec, but it based merely on observations.
        if (gotInitialPlayState
        && !value.isNull()
        &&  MissionControl::acquire()->metadataWatcher()->metadata().value(MAFW_METADATA_KEY_VIDEO_CODEC).isNull()
#ifdef MAFW_WORKAROUNDS
        // Apparently, the problem described below affects also some local files.
        // Try to solve that by disabling the detection for all local files.
        && !currentObjectId.startsWith("localtagfs::")
        // Looks like the renderer cannot tell us the video codec in RTSP streams.
        // Being able to play something without knowing the codec smells fishy,
        // so I take it for a bug in MAFW and I'm putting this as a workaround.
        && !currentObjectId.startsWith("urisource::rtsp://"))
#endif
        {
            qDebug() << "Video codec info unavailable, switching to radio mode";

            // The stream has been identified as audio-only, which means that the radio
            // window is a more suitable option. To not lose the current playlist,
            // the transition will happen by deleting the radio playlist and renaming
            // the video playlist to radio playlist.
            MafwPlaylistManagerAdapter *playlistManager = MafwPlaylistManagerAdapter::get();
            playlistManager->deletePlaylist("FmpRadioPlaylist");
            mafw_playlist_set_name(MAFW_PLAYLIST(playlistManager->createPlaylist("FmpVideoPlaylist")), "FmpRadioPlaylist");

            RadioNowPlayingWindow *window = new RadioNowPlayingWindow(this->parentWidget(), mafwFactory);

            // The video window will be closed because the radio window took over,
            // so don't stop/save the playback state in this case.
            saveStateOnClose = false;

            delete this;

            window->show();
        }
    }

    // Try to perform fit-to-screen if the necessary info has arrived
    else if (key == MAFW_METADATA_KEY_RES_X) {
        videoWidth = value.toInt();
        setFitToScreen(fitToScreen);
    }
    else if (key == MAFW_METADATA_KEY_RES_Y) {
        videoHeight = value.toInt();
        setFitToScreen(fitToScreen);
    }

    else if (key == MAFW_METADATA_KEY_DURATION) {
        videoLength = value.isNull() ? Duration::Unknown : value.toInt();
        ui->videoLengthLabel->setText(mmss_len(videoLength));
        ui->positionSlider->setRange(0, videoLength);
    }
    else if (key == MAFW_METADATA_KEY_IS_SEEKABLE) {
        ui->positionSlider->setEnabled(value.toBool());
    }
    else if (key == MAFW_METADATA_KEY_URI) {
        uri = value.toString();
    }
    else if (key == MAFW_METADATA_KEY_PAUSED_POSITION) {
        if (resumePosition != Duration::Blank)
            resumePosition = value.isNull() ? Duration::Unknown : value.toInt();

        qDebug() << "Position to resume from:" << resumePosition;

        // Check if things have settled down and the proper playback has started
        if (gotCurrentPlayState) {
            // We are late with fetching the paused position, but in practice it
            // is usually a few seconds at most, so restoring the saved position
            // is probably still desired.
            if (resumePosition > 0)
                mafwrenderer->setPosition(SeekAbsolute, resumePosition);
            resumePosition = Duration::Blank;
        }
    }
}

#ifdef MAFW
void VideoNowPlayingWindow::onGetStatus(MafwPlaylist*, uint index, MafwPlayState state, const char* objectId, QString error)
{
    // This is a one-time handler, so disconnect it right after it is called
    disconnect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
               this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));

    // This is the function which will take over from here
    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    connect(mafwrenderer, SIGNAL(mediaChanged(int,char*)), this, SLOT(onMediaChanged(int,char*)));

    // Forward the received info to more specific handlers
    onStateChanged(state);
    onMediaChanged(index, const_cast<char*>(objectId));

    if (!error.isEmpty())
        qDebug() << error;
}
#endif

#ifdef MAFW
void VideoNowPlayingWindow::onMediaChanged(int, char* objectId)
{
    // Store the last received object identifier as current
    currentObjectId = QString::fromUtf8(objectId);

    // Reset the last received position from the previous track
    currentPosition = 0;

    // Resuming should be enabled only for the first video played
    if (gotInitialPlayState)
        resumePosition = Duration::Blank;

    // Rearrange the UI according to the origin of the media
    if (currentObjectId.startsWith("localtagfs::")) { // local storage
        ui->bookmarkButton->hide();
        ui->shareButton->show();
        ui->deleteButton->show();
    } else { // remote sources
        ui->shareButton->hide();
        ui->deleteButton->hide();
        ui->bookmarkButton->show();
    }

    // Length of the video will be determined later, when possible
    videoLength = Duration::Unknown;

    // Start with the default size of the window
    videoWidth = videoHeight = 0;
    setFitToScreen(fitToScreen);

    // Start with buffering info hidden
    onBufferingInfo(1.0);
}
#endif

void VideoNowPlayingWindow::onPrevButtonClicked()
{
#ifdef MAFW
    if (ui->prevButton->isDown()) {
        buttonWasDown = true;
        slowRev();
    } else {
        if (!buttonWasDown) {
            if (this->currentPosition > 3) {
                mafwrenderer->setPosition(SeekAbsolute, 0);
                mafwrenderer->getPosition();
            } else {
                gotCurrentPlayState = false;
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
        slowFwd();
    } else {
        if (!buttonWasDown) {
            gotCurrentPlayState = false;
            mafwrenderer->next();
        }
        buttonWasDown = false;
    }
#endif
}

// Bookmark the video
void VideoNowPlayingWindow::onBookmarkClicked()
{
#ifdef MAFW
    mafwrenderer->pause();
    BookmarkDialog bookmarkDialog(this, mafwFactory, Media::Video, uri);
    bookmarkDialog.exec();
#endif
}

// Delete the video
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

// Share the video
void VideoNowPlayingWindow::onShareClicked()
{
    mafwrenderer->pause();
    (new ShareDialog(this, mafwSource, currentObjectId))->show();
}

// The volume slider was pressed, make sure that the volume is in sync with it
void VideoNowPlayingWindow::onVolumeSliderPressed()
{
    volumeTimer->stop();
#ifdef MAFW
    mafwrenderer->setVolume(ui->volumeSlider->value());
#endif
}

// The volume slider was released, make sure that the volume is in sync with it
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
        volumeTimer->start();
    } else {
        ui->volumeSlider->hide();
        ui->buttonWidget->show();
        volumeTimer->stop();
    }
}

void VideoNowPlayingWindow::startPositionTimer()
{
    if (!positionTimer->isActive()
    &&  mafwState == Playing
    &&  overlayVisible
    &&  !Maemo5DeviceEvents::acquire()->isScreenLocked())
    {
        mafwrenderer->getPosition();
        positionTimer->start();
    }
}

// Toggle the visibility of the settings bar
void VideoNowPlayingWindow::toggleSettings()
{
    showSettings = !showSettings;
    QSettings().setValue("Videos/showSettings", showSettings);
    ui->settingsOverlay->setVisible(showSettings);
}

// Toggle the visibility of the whole overlay
void VideoNowPlayingWindow::toggleOverlay()
{
    overlayRequestedByUser = !overlayVisible;
    showOverlay(overlayRequestedByUser);
}

#ifdef MAFW
void VideoNowPlayingWindow::onPropertyChanged(const QDBusMessage &msg)
{
    /*dbus-send --print-reply --type=method_call --dest=com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer \
                 /com/nokia/mafw/renderer/gstrenderer com.nokia.mafw.extension.get_extension_property string:volume*/
    if (msg.arguments()[0].toString() == MAFW_PROPERTY_RENDERER_VOLUME) {
        int volumeLevel = qdbus_cast<QVariant>(msg.arguments()[1]).toInt();

        ui->volumeSlider->setValue(volumeLevel);
    }
}
#endif

// Issue the play command if ready, otherwise queue it
void VideoNowPlayingWindow::play()
{
    if (gotInitialStopState) {
        mafwrenderer->play();
    } else {
        playWhenReady = true;
    }
}

#ifdef MAFW
void VideoNowPlayingWindow::onStateChanged(int state)
{
    qDebug() << "State changed:" << state;

    mafwState = state;

    // The life of this window is dependent on the playback state. Unfortunately,
    // even after issuing a stop command, getting the state of the renderer can
    // still fetch 'playing' as the result for a while. To prevent the window
    // from closing prematurely after a late state change to 'stopped', wait with
    // any decisions until the stopped state is detected.
    if (!gotInitialStopState) {
        if (state == Stopped) {
            // Mark readiness for full operation
            gotInitialStopState = true;

            // Various status notifications
            connect(mafwrenderer, SIGNAL(signalGetPosition(int,QString)), this, SLOT(onPositionChanged(int,QString)));
            connect(mafwrenderer, SIGNAL(signalGetVolume(int)), ui->volumeSlider, SLOT(setValue(int)));
            connect(mafwrenderer, SIGNAL(bufferingInfo(float)), this, SLOT(onBufferingInfo(float)));

            // Position slider
            connect(positionTimer, SIGNAL(timeout()), mafwrenderer, SLOT(getPosition()));
            connect(ui->positionSlider, SIGNAL(sliderPressed()), this, SLOT(onPositionSliderPressed()));
            connect(ui->positionSlider, SIGNAL(sliderReleased()), this, SLOT(onPositionSliderReleased()));
            connect(ui->positionSlider, SIGNAL(sliderMoved(int)), this, SLOT(onPositionSliderMoved(int)));
        } else {
            mafwrenderer->stop();
        }
    }

    if (gotInitialStopState) {
        // If in the meantime there was a request to start playback, this is the time to fulfill it
        if (playWhenReady) {
            playWhenReady = false;
            mafwrenderer->play();
        }
    } else {
        return;
    }

    if (state == Playing) {
        gotInitialPlayState = true;
        gotCurrentPlayState = true;
    }

    if (state == Transitioning) {
        // Discard state information for the track that was playing a moment ago
        if (!playedObjectId.isEmpty()) {
            GHashTable* metadata = mafw_metadata_new();
            mafw_metadata_add_int(metadata, MAFW_METADATA_KEY_PAUSED_POSITION, 0);
            mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI, "");
            mafwSource->setMetadata(playedObjectId.toUtf8(), metadata);
            mafw_metadata_release(metadata);
        }

        // If the renderer is transitioning, it means that another video will be
        // played in a moment. If the continuous mode is not enabled, this is
        // the time when the next video should be prevented from playing. This
        // is simply done by issuing the stop command, but as the transition is
        // already taking place, the renderer will end up on the next item. To
        // correct this, we have to move back by one item.
        if (gotCurrentPlayState && !QSettings().value("Videos/continuousPlayback", false).toBool()) {
            mafwrenderer->stop();
            mafwrenderer->previous();
        }

        // Reset position display
        ui->positionSlider->setEnabled(false);
        ui->positionSlider->setValue(0);
        ui->currentPositionLabel->setText(mmss_pos(0));

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
            // The play button becomes a resume button
            ui->playButton->setIcon(QIcon(playButtonIcon));
            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
            connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(resume()));

#ifdef Q_WS_MAEMO_5
            // Popups allowed
            this->setDNDAtom(false);
#endif

            // Playback position is not emitted in the paused state. Request it
            // manually to get the exact value.
            mafwrenderer->getPosition();

            positionTimer->stop();
        }
        else if (state == Playing) {
            // The play button becomes a pause button
            ui->playButton->setIcon(QIcon(pauseButtonIcon));
            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
            connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(pause()));

#ifdef Q_WS_MAEMO_5
            // Popups disallowed
            this->setDNDAtom(true);
#endif

            // The resume position should be touched only if proper metadata has
            // already arrived.
            if (resumePosition != Duration::Unknown) {
                if (resumePosition > 0)
                    mafwrenderer->setPosition(SeekAbsolute, resumePosition);
                resumePosition = Duration::Blank;
            }

            // Remember the last object which was actually playing
            playedObjectId = currentObjectId;

            mafwrenderer->getPosition();

            startPositionTimer();
        }
        else if (state == Stopped) {
            // The play button becomes itself
            ui->playButton->setIcon(QIcon(playButtonIcon));
            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
            connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(play()));

#ifdef Q_WS_MAEMO_5
            // Popups allowed
            this->setDNDAtom(false);
#endif

            // Reset the last received position
            currentPosition = 0;

            positionTimer->stop();

            if (gotCurrentPlayState)
                delete this; // why is it not deleted automatically, despite WA_DeleteOnClose?
        }
    }
}
#endif

void VideoNowPlayingWindow::setFitToScreen(bool enable)
{
    // Store the requested behavior
    fitToScreen = enable;
    QSettings().setValue("Videos/fitToScreen", enable);

    int w, h;

    // Some sizes cause wrong colors, so it's safer to use constant values which
    // are known to work. The multiplier of 2 will allow up to 50% of the image
    // to be outside of the screen.
    if (enable && videoWidth && videoHeight) {
        if((float) videoWidth / videoHeight > 800.0 / 480.0) {
            w = 800 * 2;
            h = 480;
        } else {
            w = 800;
            h = 480 * 2;
        }
    } else {
        w = 800;
        h = 480;
    }

    if (w != ui->videoWidget->width() || h != ui->videoWidget->height())
        ui->videoWidget->setGeometry((800-w)/2, (480-h)/2, w, h);
}

void VideoNowPlayingWindow::setContinuousPlayback(bool enable)
{
    QSettings().setValue("Videos/continuousPlayback", enable);
}

#ifdef Q_WS_MAEMO_5
// Prevent notifications and popups from obscuring the window
void VideoNowPlayingWindow::setDNDAtom(bool dnd)
{
    uchar enable = dnd;
    Atom winDNDAtom = XInternAtom(QX11Info::display(), "_HILDON_DO_NOT_DISTURB", false);
    XChangeProperty(QX11Info::display(), winId(), winDNDAtom, XA_INTEGER, 32, PropModeReplace, &enable, 1);
}
#endif

#ifdef MAFW
// Move playback forward by a small amount
void VideoNowPlayingWindow::slowFwd()
{
    mafwrenderer->setPosition(SeekRelative, 10);
    mafwrenderer->getPosition();
}

// Move playback backward by a small amount
void VideoNowPlayingWindow::slowRev()
{
    mafwrenderer->setPosition(SeekRelative, -10);
    mafwrenderer->getPosition();
}

// Move playback forward by a large amount
void VideoNowPlayingWindow::fastFwd()
{
    mafwrenderer->setPosition(SeekRelative, 60);
    mafwrenderer->getPosition();
}

// Move playback backward by a small amount
void VideoNowPlayingWindow::fastRev()
{
    mafwrenderer->setPosition(SeekRelative, -60);
    mafwrenderer->getPosition();
}
#endif

void VideoNowPlayingWindow::togglePlayback()
{
    if (this->mafwState == Playing)
        mafwrenderer->pause();
    else if (this->mafwState == Paused)
        mafwrenderer->resume();
    else if (this->mafwState == Stopped)
        mafwrenderer->play();
}

// The overlay can be toggled by clicking the main area of the window
void VideoNowPlayingWindow::mouseReleaseEvent(QMouseEvent *)
{
    toggleOverlay();
}

void VideoNowPlayingWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->isAutoRepeat()) return;

    if (e->key() == Qt::Key_Backspace)
        this->close();

    else if (e->key() == Qt::Key_Left
         ||  e->key() == Qt::Key_Right
         ||  e->key() == Qt::Key_Up
         ||  e->key() == Qt::Key_Down)
    {
        keyToRepeat = e->key();
        keyRepeatTimer->start();
        repeatKey();
    }
}

void VideoNowPlayingWindow::keyReleaseEvent(QKeyEvent *e)
{
    if (e && e->isAutoRepeat()) return;

    keyRepeatTimer->stop();
}

void VideoNowPlayingWindow::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::ActivationChange && !this->isActiveWindow())
        keyReleaseEvent(NULL);

    QMainWindow::changeEvent(e);
}

void VideoNowPlayingWindow::repeatKey()
{
    switch (keyToRepeat)
    {
        case Qt::Key_Left:  slowRev(); break;
        case Qt::Key_Right: slowFwd(); break;
        case Qt::Key_Up:    fastFwd(); break;
        case Qt::Key_Down:  fastRev(); break;
    }
}

// Set the visibility of the overlay
void VideoNowPlayingWindow::showOverlay(bool show)
{
    ui->settingsOverlay->setVisible(show && showSettings);
    ui->controlOverlay->setVisible(show);
    ui->toolbarOverlay->setVisible(show);
    ui->wmCloseButton->setVisible(show);
    ui->wmEditButton->setVisible(show);

    overlayVisible = show;

    if (show) {
        startPositionTimer();
    } else {
        positionTimer->stop();
    }
}

void VideoNowPlayingWindow::onPositionSliderPressed()
{
    onPositionSliderMoved(ui->positionSlider->value());
}

void VideoNowPlayingWindow::onPositionSliderReleased()
{
#ifdef MAFW
    mafwrenderer->setPosition(SeekAbsolute, ui->positionSlider->value());
    ui->currentPositionLabel->setText(mmss_pos(reverseTime ? ui->positionSlider->value()-videoLength :
                                                             ui->positionSlider->value()));
#endif
}

void VideoNowPlayingWindow::onPositionSliderMoved(int position)
{
    ui->currentPositionLabel->setText(mmss_pos(reverseTime ? position-videoLength : position));
#ifdef MAFW
    if (!lazySliders)
        mafwrenderer->setPosition(SeekAbsolute, position);
#endif
}

#ifdef MAFW
// Handle buffring progress changes
void VideoNowPlayingWindow::onBufferingInfo(float status)
{
    if (status == 1.0) {
        // Return the overlay to the state requested by the user
        showOverlay(overlayRequestedByUser);

        // Hide the buffer bar
        ui->bufferBar->hide();
        ui->positionWidget->show();
        ui->bufferBar->setRange(0, 0);
    } else { // status != 1.0
        int percentage = (int)(status*100);
        ui->bufferBar->setRange(0, 100);
        ui->bufferBar->setValue(percentage);
        ui->bufferBar->setFormat(tr("Buffering") + " %p%");

        // Show the buffer bar and enable the overlay each time the buffering starts
        if (ui->bufferBar->isHidden()) {
            ui->positionWidget->hide();
            ui->bufferBar->show();
            showOverlay(true);
        }
    }
}
#endif

#ifdef MAFW
// Handle changes in playback position
void VideoNowPlayingWindow::onPositionChanged(int position, QString)
{
    // Store the last received position as current
    currentPosition = position;

    // Refresh the UI only if the user is not holding the position slider
    if (!ui->positionSlider->isSliderDown() && ui->positionSlider->isVisible()) {
        ui->currentPositionLabel->setText(mmss_pos(reverseTime ? position-videoLength : position));
        if (ui->positionSlider->isEnabled())
            ui->positionSlider->setValue(position);
    }
}
#endif

#ifdef MAFW
void VideoNowPlayingWindow::onErrorOccured(const QDBusMessage &msg)
{
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
        QLabel *errorInfo = new QLabel(errorMsg);
        errorInfo->setWordWrap(true);

        QMaemo5InformationBox *errorBox = new QMaemo5InformationBox(this);
        errorBox->setAttribute(Qt::WA_DeleteOnClose);
        errorBox->setTimeout(QMaemo5InformationBox::NoTimeout);
        errorBox->setContentsMargins(90, 30, 90, 30);
        errorBox->setWidget(errorInfo);

        errorBox->exec();

        this->close();
#else
        QMessageBox::critical(this, tr("Unable to play media") + "\n", errorMsg);
#endif
    }
}
#endif
