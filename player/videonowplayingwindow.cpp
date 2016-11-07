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

VideoNowPlayingWindow::VideoNowPlayingWindow(QWidget *parent, MafwRegistryAdapter *mafwRegistry, bool overlay) :
    QMainWindow(parent),
    ui(new Ui::VideoNowPlayingWindow),
    mafwRegistry(mafwRegistry),
    mafwRenderer(mafwRegistry->renderer())
{
    ui->setupUi(this);
    setIcons();

    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_Maemo5StackedWindow);

    // Lock the orientation to landscape, but remember the policy to restore it later
    Rotator *rotator = Rotator::acquire();
    rotatorPolicy = rotator->policy();
    rotator->setPolicy(Rotator::Landscape);

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

    mafwState = Transitioning;

    overlayRequestedByUser = overlay;
    showOverlay(overlay);

    connectSignals();

    // Metadata
    MetadataWatcher *mw = MissionControl::acquire()->metadataWatcher();
    connect(mw, SIGNAL(metadataChanged(QString,QVariant)), this, SLOT(onMetadataChanged(QString,QVariant)));
    QMap<QString,QVariant> metadata = mw->metadata();
    onMetadataChanged(MAFW_METADATA_KEY_VIDEO_CODEC, metadata.value(MAFW_METADATA_KEY_VIDEO_CODEC));
    onMetadataChanged(MAFW_METADATA_KEY_AUDIO_CODEC, metadata.value(MAFW_METADATA_KEY_AUDIO_CODEC));
    onMetadataChanged(MAFW_METADATA_KEY_DURATION, metadata.value(MAFW_METADATA_KEY_DURATION));
    onMetadataChanged(MAFW_METADATA_KEY_IS_SEEKABLE, metadata.value(MAFW_METADATA_KEY_IS_SEEKABLE));
    onMetadataChanged(MAFW_METADATA_KEY_PAUSED_POSITION, metadata.value(MAFW_METADATA_KEY_PAUSED_POSITION));
    onMetadataChanged(MAFW_METADATA_KEY_RES_X, metadata.value(MAFW_METADATA_KEY_RES_X));
    onMetadataChanged(MAFW_METADATA_KEY_RES_Y, metadata.value(MAFW_METADATA_KEY_RES_Y));
    onMetadataChanged(MAFW_METADATA_KEY_URI, metadata.value(MAFW_METADATA_KEY_URI));
    mafwSource = mw->currentSource();

    ui->currentPositionLabel->installEventFilter(this);

    // Set up video surface
    QApplication::syncX();
    mafwRenderer->setColorKey(colorKey().rgb() & 0xffffff);
    mafwRenderer->setXid(ui->videoWidget->winId());

    // Do not jump to the next playlist item if an error occurs
    mafwRenderer->setErrorPolicy(MAFW_RENDERER_ERROR_POLICY_STOP);

    // Request some initial values
    mafwRenderer->getStatus();
    mafwRenderer->getVolume();
}

VideoNowPlayingWindow::~VideoNowPlayingWindow()
{
    if (saveStateOnClose) {
        // The state should be saved only if resuming is already disabled,
        // to prevent overwriting a position that is waiting to be resumed.
        if (resumePosition == Duration::Blank) {
            // Pausing the video should cause a thumbnail to be generated
            if (mafwState == Playing)
                mafwRenderer->pause();

            // Prepare a metadata table to save the state in it
            GHashTable* metadata = mafw_metadata_new();

            qDebug() << "Saving position" << currentPosition << "for" << playedObjectId;

            // Store the current position so that the playback can be resumed later
            mafw_metadata_add_int(metadata, MAFW_METADATA_KEY_PAUSED_POSITION, currentPosition);

            // If the position is at the beginning, the pause thumbnail should be reset
            if (currentPosition == 0)
                mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI, "");

            // Commit the metadata in the table
            mafwSource->setMetadata(playedObjectId, metadata);
            mafw_metadata_release(metadata);
        }

        // Make sure that video playback is dead after the window is closed,
        // because otherwise ghost windows may appear, rapidly opening and closing.
        mafwRenderer->stop();
        // NOTE: If ghost windows happen despite the line above, it might be
        // necessary to wait for a confirmation that the playback is stopped and
        // only then close the window.
    }

    // Restore the default error policy
    mafwRenderer->setErrorPolicy(MAFW_RENDERER_ERROR_POLICY_CONTINUE);

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

    ui->detailsButton->setIcon(QIcon::fromTheme(detailsButtonIcon));
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
    connect(ui->detailsButton, SIGNAL(clicked()), this, SLOT(onDetailsClicked()));
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
    connect(ui->volumeSlider, SIGNAL(sliderMoved(int)), mafwRenderer, SLOT(setVolume(int)));
    connect(volumeTimer, SIGNAL(timeout()), this, SLOT(toggleVolumeSlider()));

    // A shortcut to toggle the overlay
    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), this, SLOT(toggleOverlay()));

    // Shortcuts to control the playback
    shortcut = new QShortcut(QKeySequence(Qt::Key_Space), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), this, SLOT(togglePlayback()));
    connect(keyRepeatTimer, SIGNAL(timeout()), this, SLOT(repeatKey()));

    // Screen locking
    connect(Maemo5DeviceEvents::acquire(), SIGNAL(screenLocked(bool)), this, SLOT(onScreenLocked(bool)));

    // Initial status
    connect(mafwRenderer, SIGNAL(statusReceived(MafwPlaylist*,uint,MafwPlayState,QString,QString)),
            this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState,QString)));

    connect(mafwRenderer, SIGNAL(propertyChanged(QString,QVariant)), this, SLOT(onPropertyChanged(QString,QVariant)));
    connect(mafwRenderer, SIGNAL(error(uint,int,QString)), this, SLOT(onErrorOccurred(uint,int,QString)));
}

void VideoNowPlayingWindow::switchToRadio()
{
#ifdef MAFW_WORKAROUNDS
    // Apparently, the problem described below affects also some local files.
    // Try to solve that by disabling the detection for all local files.
    if (currentObjectId.startsWith("localtagfs::")
    // Looks like the renderer cannot tell us the video codec in RTSP streams.
    // Being able to play something without knowing the codec smells fishy,
    // so I take it for a bug in MAFW and I'm putting this as a workaround.
    ||  currentObjectId.startsWith("urisource::rtsp://"))
        return;
#endif

    qDebug() << "Video codec info unavailable, switching to radio mode";

    // The stream has been identified as audio-only, which means that the radio
    // window is a more suitable option. To not lose the current playlist,
    // the transition will happen by deleting the radio playlist and renaming
    // the video playlist to radio playlist.
    MafwPlaylistManagerAdapter::get()->deletePlaylist("FmpRadioPlaylist");
    MafwPlaylistAdapter("FmpVideoPlaylist").setName("FmpRadioPlaylist");

    RadioNowPlayingWindow *window = new RadioNowPlayingWindow(this->parentWidget(), mafwRegistry);

    // The video window will be closed because the radio window took over,
    // so don't stop/save the playback state in this case.
    saveStateOnClose = false;

    delete this;

    window->show();
}

void VideoNowPlayingWindow::onMetadataChanged(QString key, QVariant value)
{
    // Try to detect whether we have a video or an audio-only stream. If not
    // sure, take the safe approach by keeping the video window open.
    if (key == MAFW_METADATA_KEY_AUDIO_CODEC) {
        if (gotInitialPlayState
        && !value.isNull()
        &&  MissionControl::acquire()->metadataWatcher()->metadata().value(MAFW_METADATA_KEY_VIDEO_CODEC).isNull())
        {
            switchToRadio();
        }
    }
    else if (key == MAFW_METADATA_KEY_VIDEO_CODEC) {
        if (gotInitialPlayState
        &&  key.isNull()
        && !MissionControl::acquire()->metadataWatcher()->metadata().value(MAFW_METADATA_KEY_AUDIO_CODEC).isNull())
        {
            switchToRadio();
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
        isMediaSeekable = value.toBool();

        if (mafwState == Playing || mafwState == Paused)
            ui->positionSlider->setEnabled(isMediaSeekable);
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
                mafwRenderer->setPosition(SeekAbsolute, resumePosition);
            resumePosition = Duration::Blank;
        }
    }
}

void VideoNowPlayingWindow::onStatusReceived(MafwPlaylist *, uint index, MafwPlayState state, QString objectId)
{
    // This is a one-time handler, so disconnect it right after it is called
    disconnect(mafwRenderer, SIGNAL(statusReceived(MafwPlaylist*,uint,MafwPlayState,QString,QString)),
               this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState,QString)));

    // This is the function which will take over from here
    connect(mafwRenderer, SIGNAL(stateChanged(MafwPlayState)), this, SLOT(onStateChanged(MafwPlayState)));
    connect(mafwRenderer, SIGNAL(mediaChanged(int,QString)), this, SLOT(onMediaChanged(int,QString)));

    // Forward the received info to more specific handlers
    onStateChanged(state);
    onMediaChanged(index, objectId);
}

void VideoNowPlayingWindow::onMediaChanged(int, QString objectId)
{
    // Store the last received object identifier as current
    currentObjectId = objectId;

    // Reset the last received position from the previous track
    currentPosition = 0;

    // Resuming should be enabled only for the first video played
    if (gotInitialPlayState)
        resumePosition = Duration::Blank;

    // Rearrange the UI according to the origin of the media
    if (objectId.startsWith("localtagfs::")) { // local storage
        ui->bookmarkButton->hide();
        ui->shareButton->show();
        ui->deleteButton->show();
    } else { // remote sources
        ui->shareButton->hide();
        ui->deleteButton->hide();
        ui->bookmarkButton->show();
    }

    // Start with buffering info hidden
    onBufferingInfo(1.0);
}

void VideoNowPlayingWindow::onPrevButtonClicked()
{
    if (ui->prevButton->isDown()) {
        buttonWasDown = true;
        slowRev();
    } else {
        if (!buttonWasDown) {
            if (this->currentPosition > 3) {
                mafwRenderer->setPosition(SeekAbsolute, 0);
            } else {
                gotCurrentPlayState = false;
                mafwRenderer->previous();
            }
        }
        buttonWasDown = false;
    }
}

void VideoNowPlayingWindow::onNextButtonClicked()
{
    if (ui->nextButton->isDown()) {
        buttonWasDown = true;
        slowFwd();
    } else {
        if (!buttonWasDown) {
            gotCurrentPlayState = false;
            mafwRenderer->next();
        }
        buttonWasDown = false;
    }
}

// Show media details
void VideoNowPlayingWindow::onDetailsClicked()
{
    mafwRenderer->pause();
    (new MetadataDialog(this, MissionControl::acquire()->metadataWatcher()->metadata()))->show();
}

// Bookmark the video
void VideoNowPlayingWindow::onBookmarkClicked()
{
    mafwRenderer->pause();
    BookmarkDialog(this, mafwRegistry, Media::Video, uri).exec();
}

// Delete the video
void VideoNowPlayingWindow::onDeleteClicked()
{
    mafwRenderer->pause();

    if (ConfirmDialog(ConfirmDialog::DeleteVideo, this).exec() == QMessageBox::Yes) {
        mafwSource->destroyObject(currentObjectId);
        this->close();
    }
}

// Share the video
void VideoNowPlayingWindow::onShareClicked()
{
    mafwRenderer->pause();
    (new ShareDialog(this, mafwSource, currentObjectId))->show();
}

// The volume slider was pressed, make sure that the volume is in sync with it
void VideoNowPlayingWindow::onVolumeSliderPressed()
{
    volumeTimer->stop();
    mafwRenderer->setVolume(ui->volumeSlider->value());
}

// The volume slider was released, make sure that the volume is in sync with it
void VideoNowPlayingWindow::onVolumeSliderReleased()
{
    volumeTimer->start();
    mafwRenderer->setVolume(ui->volumeSlider->value());
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
        mafwRenderer->getPosition();
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

void VideoNowPlayingWindow::onPropertyChanged(const QString &name, const QVariant &value)
{
    if (name == MAFW_PROPERTY_RENDERER_VOLUME && !ui->volumeSlider->isSliderDown())
        ui->volumeSlider->setValue(value.toInt());
}

// Issue the play command if ready, otherwise queue it
void VideoNowPlayingWindow::play()
{
    if (gotInitialStopState) {
        mafwRenderer->play();
    } else {
        playWhenReady = true;
    }
}

void VideoNowPlayingWindow::onStateChanged(MafwPlayState state)
{
    qDebug() << "State changed:" << state;

    mafwState = state;

    updateDNDAtom();

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
            connect(mafwRenderer, SIGNAL(positionReceived(int,QString)), this, SLOT(onPositionChanged(int)));
            connect(mafwRenderer, SIGNAL(volumeReceived(int,QString)), ui->volumeSlider, SLOT(setValue(int)));
            connect(mafwRenderer, SIGNAL(bufferingInfo(float)), this, SLOT(onBufferingInfo(float)));

            // Position slider
            connect(positionTimer, SIGNAL(timeout()), mafwRenderer, SLOT(getPosition()));
            connect(ui->positionSlider, SIGNAL(sliderPressed()), this, SLOT(onPositionSliderPressed()));
            connect(ui->positionSlider, SIGNAL(sliderReleased()), this, SLOT(onPositionSliderReleased()));
            connect(ui->positionSlider, SIGNAL(sliderMoved(int)), this, SLOT(onPositionSliderMoved(int)));
        } else {
            mafwRenderer->stop();
        }
    }

    if (gotInitialStopState) {
        // If in the meantime there was a request to start playback, this is the time to fulfill it
        if (playWhenReady) {
            playWhenReady = false;
            mafwRenderer->play();
        }
    } else {
        return;
    }

    if (state == Transitioning) {
        // Discard state information for the track that was playing a moment ago
        if (!playedObjectId.isEmpty()) {
            GHashTable* metadata = mafw_metadata_new();
            mafw_metadata_add_int(metadata, MAFW_METADATA_KEY_PAUSED_POSITION, 0);
            mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI, "");
            mafwSource->setMetadata(playedObjectId, metadata);
            mafw_metadata_release(metadata);
        }

        // If the renderer is transitioning, it means that another video will be
        // played in a moment. If the continuous mode is not enabled, this is
        // the time when the next video should be prevented from playing. This
        // is simply done by issuing the stop command, but as the transition is
        // already taking place, the renderer will end up on the next item. To
        // correct this, we have to move back by one item.
        if (gotCurrentPlayState && !QSettings().value("Videos/continuousPlayback", false).toBool()) {
            mafwRenderer->stop();
            mafwRenderer->previous();
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
            ui->positionSlider->setEnabled(isMediaSeekable);

            // The play button becomes a resume button
            ui->playButton->setIcon(QIcon(playButtonIcon));
            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
            connect(ui->playButton, SIGNAL(clicked()), mafwRenderer, SLOT(resume()));

            // Playback position is not emitted in the paused state. Request it
            // manually to get the exact value.
            mafwRenderer->getPosition();
            positionTimer->stop();
        }
        else if (state == Playing) {
            gotInitialPlayState = true;
            gotCurrentPlayState = true;

            ui->positionSlider->setEnabled(isMediaSeekable);

            // The play button becomes a pause button
            ui->playButton->setIcon(QIcon(pauseButtonIcon));
            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
            connect(ui->playButton, SIGNAL(clicked()), mafwRenderer, SLOT(pause()));

            // The resume position should be touched only if proper metadata has
            // already arrived.
            if (resumePosition != Duration::Unknown) {
                if (resumePosition > 0)
                    mafwRenderer->setPosition(SeekAbsolute, resumePosition);
                resumePosition = Duration::Blank;
            }

            // Remember the last object which was actually playing
            playedObjectId = currentObjectId;

            mafwRenderer->getPosition();
            startPositionTimer();
        }
        else if (state == Stopped) {
            // The play button becomes itself
            ui->playButton->setIcon(QIcon(playButtonIcon));
            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
            connect(ui->playButton, SIGNAL(clicked()), mafwRenderer, SLOT(play()));

            // Reset the last received position
            currentPosition = 0;

            positionTimer->stop();

            if (gotCurrentPlayState)
                delete this; // why is it not deleted automatically, despite WA_DeleteOnClose?
        }
    }
}

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

// Prevent notifications and popups from obscuring the window
void VideoNowPlayingWindow::updateDNDAtom()
{
    Atom atom = XInternAtom(QX11Info::display(), "_HILDON_DO_NOT_DISTURB", false);

    if (uchar enable = !overlayVisible && mafwState == Playing) {
        XChangeProperty(QX11Info::display(), winId(), atom, XA_INTEGER, 32, PropModeReplace, &enable, 1);
    } else {
        XDeleteProperty(QX11Info::display(), winId(), atom);
    }
}

// Move playback forward by a small amount
void VideoNowPlayingWindow::slowFwd()
{
    mafwRenderer->setPosition(SeekRelative, 10);
}

// Move playback backward by a small amount
void VideoNowPlayingWindow::slowRev()
{
    mafwRenderer->setPosition(SeekRelative, -10);
}

// Move playback forward by a large amount
void VideoNowPlayingWindow::fastFwd()
{
    mafwRenderer->setPosition(SeekRelative, 60);
}

// Move playback backward by a small amount
void VideoNowPlayingWindow::fastRev()
{
    mafwRenderer->setPosition(SeekRelative, -60);
}

void VideoNowPlayingWindow::togglePlayback()
{
    if (this->mafwState == Playing)
        mafwRenderer->pause();
    else if (this->mafwState == Paused)
        mafwRenderer->resume();
    else if (this->mafwState == Stopped)
        mafwRenderer->play();
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

    updateDNDAtom();

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
    mafwRenderer->setPosition(SeekAbsolute, ui->positionSlider->value());
    ui->currentPositionLabel->setText(mmss_pos(reverseTime ? ui->positionSlider->value()-videoLength :
                                                             ui->positionSlider->value()));
}

void VideoNowPlayingWindow::onPositionSliderMoved(int position)
{
    ui->currentPositionLabel->setText(mmss_pos(reverseTime ? position-videoLength : position));
    if (!lazySliders)
        mafwRenderer->setPosition(SeekAbsolute, position);
}

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

// Handle changes in playback position
void VideoNowPlayingWindow::onPositionChanged(int position)
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

void VideoNowPlayingWindow::onErrorOccurred(uint domain, int code, const QString &message)
{
    if (domain == MAFW_RENDERER_ERROR) {
        QLabel *errorInfo = new QLabel(tr("Unable to play media") + "\n" + errorMessage(code, message));
        errorInfo->setWordWrap(true);

        QMaemo5InformationBox *errorBox = new QMaemo5InformationBox(this);
        errorBox->setAttribute(Qt::WA_DeleteOnClose);
        errorBox->setTimeout(QMaemo5InformationBox::NoTimeout);
        errorBox->setContentsMargins(90, 30, 90, 30);
        errorBox->setWidget(errorInfo);

        errorBox->exec();

        this->close();
    }
}

QString VideoNowPlayingWindow::errorMessage(int code, const QString &message) {
    switch (code) {
        case MAFW_RENDERER_ERROR_NO_MEDIA:
            return tr("Media not found");
        case MAFW_RENDERER_ERROR_URI_NOT_AVAILABLE:
            return tr("URI not available");
        case MAFW_RENDERER_ERROR_INVALID_URI:
            return tr("Invalid URI");
        case MAFW_RENDERER_ERROR_MEDIA_NOT_FOUND:
            return tr("Unable to open media");
        case MAFW_RENDERER_ERROR_STREAM_DISCONNECTED:
            return tr("Playback stream no longer available");
        case MAFW_RENDERER_ERROR_TYPE_NOT_AVAILABLE:
            return tr("Could not determine MIME-type");
        case MAFW_RENDERER_ERROR_PLAYBACK:
            return tr("General error occured, unable to continue playback");
        case MAFW_RENDERER_ERROR_UNABLE_TO_PERFORM:
            return tr("General error occured");
        case MAFW_RENDERER_ERROR_UNSUPPORTED_TYPE:
            return tr("Unsupported media");
        case MAFW_RENDERER_ERROR_UNSUPPORTED_RESOLUTION:
            return tr("Unsupported resolution");
        case MAFW_RENDERER_ERROR_UNSUPPORTED_FPS:
            return tr("Unsupported framerate");
        case MAFW_RENDERER_ERROR_DRM:
            return tr("Media is protected by DRM");
        case MAFW_RENDERER_ERROR_DEVICE_UNAVAILABLE:
            return tr("System sound device is unavailable");
        case MAFW_RENDERER_ERROR_CORRUPTED_FILE:
            return tr("Media corrupted");
        case MAFW_RENDERER_ERROR_PLAYLIST_PARSING:
            return tr("Error while parsing playlist") + "\n" + tr("Playlist may be corrupt or empty");
        case MAFW_RENDERER_ERROR_CODEC_NOT_FOUND:
            return tr("Codec not found:") + "\n" + message;
        case MAFW_RENDERER_ERROR_VIDEO_CODEC_NOT_FOUND:
            return tr("Video codec not found:") + "\n" + message;
        case MAFW_RENDERER_ERROR_AUDIO_CODEC_NOT_FOUND:
            return tr("Audio codec not found:") + "\n" + message;
        case MAFW_RENDERER_ERROR_NO_PLAYLIST:
            return tr("No playlist assigned");
        case MAFW_RENDERER_ERROR_INDEX_OUT_OF_BOUNDS:
            return tr("Media index is not in bound with playlist items");
        case MAFW_RENDERER_ERROR_CANNOT_PLAY:
            return tr("Unable to start playback");
        case MAFW_RENDERER_ERROR_CANNOT_STOP:
            return tr("Unable to stop playback");
        case MAFW_RENDERER_ERROR_CANNOT_PAUSE:
            return tr("Unable to pause playback");
        case MAFW_RENDERER_ERROR_CANNOT_SET_POSITION:
            return tr("Unable to seek position in media");
        case MAFW_RENDERER_ERROR_CANNOT_GET_POSITION:
            return tr("Unable to retrieve current position in media");
        case MAFW_RENDERER_ERROR_CANNOT_GET_STATUS:
            return tr("Unable to get current playback status");
        default:
            return message;
    }
}
