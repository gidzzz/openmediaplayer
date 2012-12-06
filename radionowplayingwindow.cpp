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

#include "radionowplayingwindow.h"

RadioNowPlayingWindow::RadioNowPlayingWindow(QWidget *parent, MafwAdapterFactory *factory) :
    BaseWindow(parent),
    ui(new Ui::RadioNowPlayingWindow)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwSource(factory->getTempSource()),
    playlist(factory->getPlaylistAdapter())
#endif
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);

    QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");
#else
    QColor secondaryColor(156, 154, 156);
#endif
    ui->songLabel->setStyleSheet(QString("color: rgb(%1, %2, %3);")
                                 .arg(secondaryColor.red())
                                 .arg(secondaryColor.green())
                                 .arg(secondaryColor.blue()));

    albumArtScene = new QGraphicsScene(ui->albumArtView);
    setAlbumImage(defaultRadioImage);

    lazySliders = QSettings().value("main/lazySliders").toBool();

    volumeTimer = new QTimer(this);
    volumeTimer->setInterval(3000);

    positionTimer = new QTimer(this);
    positionTimer->setInterval(1000);

    this->updateSongLabel();
    this->setIcons();
    this->connectSignals();

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    orientationChanged(rotator->width(), rotator->height());

#ifdef MAFW
    mafwrenderer->getStatus();
    mafwrenderer->getPosition();
    mafwrenderer->getVolume();
#endif
}

RadioNowPlayingWindow::~RadioNowPlayingWindow()
{
    delete ui;
}

void RadioNowPlayingWindow::connectSignals()
{
    QShortcut *shortcut;

    shortcut = new QShortcut(QKeySequence(Qt::Key_Space), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), this, SLOT(togglePlayback()));
    shortcut = new QShortcut(QKeySequence(Qt::Key_Left), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), mafwrenderer, SLOT(previous()));
    shortcut = new QShortcut(QKeySequence(Qt::Key_Right), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), mafwrenderer, SLOT(next()));

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Space), this); // prevent Ctrl+Space from toggling playback

#ifdef Q_WS_MAEMO_5
    connect(ui->actionFM_transmitter, SIGNAL(triggered()), this, SLOT(showFMTXDialog()));
#endif
    connect(ui->actionAdd_radio_bookmark, SIGNAL(triggered()), this, SLOT(showBookmarkDialog()));

    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(volumeWatcher()));
    connect(volumeTimer, SIGNAL(timeout()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeSlider, SIGNAL(sliderPressed()), this, SLOT(onVolumeSliderPressed()));
    connect(ui->volumeSlider, SIGNAL(sliderReleased()), this, SLOT(onVolumeSliderReleased()));

    connect(ui->nextButton, SIGNAL(pressed()), this, SLOT(onNextButtonPressed()));
    connect(ui->nextButton, SIGNAL(released()), this, SLOT(onNextButtonPressed()));
    connect(ui->prevButton, SIGNAL(pressed()), this, SLOT(onPrevButtonPressed()));
    connect(ui->prevButton, SIGNAL(released()), this, SLOT(onPrevButtonPressed()));
#ifdef MAFW
    connect(ui->playButton, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onPlayMenuRequested(QPoint)));
    connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(onNextButtonClicked()));
    connect(ui->prevButton, SIGNAL(clicked()), this, SLOT(onPreviousButtonClicked()));

    connect(ui->volumeSlider, SIGNAL(sliderMoved(int)), mafwrenderer, SLOT(setVolume(int)));
    connect(ui->positionSlider, SIGNAL(sliderPressed()), this, SLOT(onPositionSliderPressed()));
    connect(ui->positionSlider, SIGNAL(sliderReleased()), this, SLOT(onPositionSliderReleased()));
    connect(ui->positionSlider, SIGNAL(sliderMoved(int)), this, SLOT(onPositionSliderMoved(int)));
    connect(positionTimer, SIGNAL(timeout()), mafwrenderer, SLOT(getPosition()));

    connect(mafwSource, SIGNAL(signalMetadataResult(QString,GHashTable*,QString)),
            this, SLOT(onSourceMetadataRequested(QString,GHashTable*,QString)));

    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    connect(mafwrenderer, SIGNAL(mediaChanged(int,char*)), this, SLOT(onMediaChanged(int,char*)));
    connect(mafwrenderer, SIGNAL(signalGetVolume(int)), ui->volumeSlider, SLOT(setValue(int)));
    connect(mafwrenderer, SIGNAL(signalGetPosition(int,QString)), this, SLOT(onGetPosition(int,QString)));
    connect(mafwrenderer, SIGNAL(bufferingInfo(float)), this, SLOT(onBufferingInfo(float)));
    connect(mafwrenderer, SIGNAL(metadataChanged(QString,QVariant)), this, SLOT(onRendererMetadataChanged(QString,QVariant)));
    connect(mafwrenderer, SIGNAL(signalGetCurrentMetadata(GHashTable*,QString,QString)),
            this, SLOT(onRendererMetadataRequested(GHashTable*,QString,QString)));
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));

    QDBusConnection::sessionBus().connect("com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer",
                                          "/com/nokia/mafw/renderer/gstrenderer",
                                          "com.nokia.mafw.extension",
                                          "property_changed",
                                          this, SLOT(onPropertyChanged(const QDBusMessage &)));
#endif
}

#ifdef MAFW
void RadioNowPlayingWindow::onPropertyChanged(const QDBusMessage &msg)
{
    if (msg.arguments()[0].toString() == "volume") {
        if (!ui->volumeSlider->isSliderDown())
            ui->volumeSlider->setValue(qdbus_cast<QVariant>(msg.arguments()[1]).toInt());
    }
}
#endif

void RadioNowPlayingWindow::setIcons()
{
    ui->prevButton->setIcon(QIcon(prevButtonIcon));
    ui->playButton->setIcon(QIcon(playButtonIcon));
    ui->nextButton->setIcon(QIcon(nextButtonIcon));
    ui->volumeButton->setIcon(QIcon(volumeButtonIcon));
}

void RadioNowPlayingWindow::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Backspace:
            this->close();
            break;

        case Qt::Key_S:
            mafwrenderer->stop();
            break;
    }
}

void RadioNowPlayingWindow::toggleVolumeSlider()
{
    if (ui->volumeSlider->isHidden()) {
        ui->buttonsWidget->hide();
        ui->volumeSlider->show();
    } else {
        ui->volumeSlider->hide();
        ui->buttonsWidget->show();
        if (volumeTimer->isActive())
            volumeTimer->stop();
    }
}

void RadioNowPlayingWindow::volumeWatcher()
{
    if (!ui->volumeSlider->isHidden())
        volumeTimer->start();
}

void RadioNowPlayingWindow::onVolumeSliderPressed()
{
    volumeTimer->stop();
#ifdef MAFW
    mafwrenderer->setVolume(ui->volumeSlider->value());
#endif
}

void RadioNowPlayingWindow::onVolumeSliderReleased()
{
    volumeTimer->start();
#ifdef MAFW
    mafwrenderer->setVolume(ui->volumeSlider->value());
#endif
}

#ifdef MAFW
void RadioNowPlayingWindow::onStateChanged(int state)
{
    mafwState = state;

    if (state == Transitioning) {
        ui->positionSlider->setEnabled(false);
        ui->positionSlider->setValue(0);
        ui->positionSlider->setRange(0, 99);
        ui->currentPositionLabel->setText("00:00");

        if (ui->bufferBar->maximum() == 0) {
            ui->seekWidget->hide();
            ui->bufferBar->show();
        }
    } else {
        if (ui->bufferBar->maximum() == 0) {
            ui->bufferBar->hide();
            ui->seekWidget->show();
        }

        if (state == Paused) {
            ui->playButton->setIcon(QIcon(playButtonIcon));
            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
            connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(resume()));
            disconnect(ui->playButton, SIGNAL(pressed()), this, SLOT(onStopButtonPressed()));
            disconnect(ui->playButton, SIGNAL(released()), this, SLOT(onStopButtonPressed()));
            mafwrenderer->getPosition();
            if (positionTimer->isActive())
                positionTimer->stop();
        }
        else if (state == Playing) {
            ui->playButton->setIcon(QIcon(pauseButtonIcon));
            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
            connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(pause()));
            disconnect(ui->playButton, SIGNAL(pressed()), this, SLOT(onStopButtonPressed()));
            disconnect(ui->playButton, SIGNAL(released()), this, SLOT(onStopButtonPressed()));
            mafwrenderer->getPosition();
            if (!positionTimer->isActive())
                positionTimer->start();
        }
        else if (state == Stopped) {
            ui->playButton->setIcon(QIcon(playButtonIcon));
            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
            connect(ui->playButton, SIGNAL(clicked()), this, SLOT(play()));
            disconnect(ui->playButton, SIGNAL(pressed()), this, SLOT(onStopButtonPressed()));
            disconnect(ui->playButton, SIGNAL(released()), this, SLOT(onStopButtonPressed()));
            if (positionTimer->isActive())
                positionTimer->stop();
        }
    }

}

void RadioNowPlayingWindow::play()
{
    QNetworkConfigurationManager manager;
    QNetworkSession session(manager.defaultConfiguration());
    if (!session.isOpen()) {
        session.open();
        session.waitForOpened();
    }
    mafwrenderer->play();
}

void RadioNowPlayingWindow::onMediaChanged(int, char* objectId)
{
    ui->stationLabel->setText(tr("(unknown station)"));

    currentObjectId = QString::fromUtf8(objectId);
    mafwSource->setSource(mafwFactory->getTempSource()->getSourceByUUID(currentObjectId.left(currentObjectId.indexOf("::"))));
    mafwSource->getMetadata(objectId, MAFW_SOURCE_LIST (MAFW_METADATA_KEY_TITLE,
                                                        MAFW_METADATA_KEY_URI));

    mafwrenderer->getCurrentMetadata();

    onBufferingInfo(1.0);
}

void RadioNowPlayingWindow::onRendererMetadataChanged(QString name, QVariant value)
{
    qDebug() << "Metadata changed:" << name << "=" << value;

    if (name == MAFW_METADATA_KEY_IS_SEEKABLE) {
        ui->positionSlider->setEnabled(value.toBool());
        this->streamIsSeekable(value.toBool());
    }
    else if (name == MAFW_METADATA_KEY_ARTIST) {
        this->artist = value.toString();
        this->updateSongLabel();
    }
    else if (name == MAFW_METADATA_KEY_TITLE) {
        this->title = value.toString();
        this->updateSongLabel();
    }
    else if (name == MAFW_METADATA_KEY_DURATION) {
        ui->streamLengthLabel->setText(time_mmss(value.toInt()));
    }
    else if (name == MAFW_METADATA_KEY_RENDERER_ART_URI) {
        setAlbumImage(value.toString());
    }
}

void RadioNowPlayingWindow::onGetStatus(MafwPlaylist*, uint index, MafwPlayState state, const char *objectId, QString)
{
    onStateChanged(state);
    onMediaChanged(index, const_cast<char*>(objectId));
}

void RadioNowPlayingWindow::updateSongLabel()
{
    QString labelText = this->title;

    if (title.isEmpty())
        labelText = tr("(unknown artist)") + " / " + tr("(unknown song)");
    else if (!artist.isEmpty())
        labelText.prepend(artist + " / ");

    ui->songLabel->setText(QFontMetrics(ui->songLabel->font()).elidedText(labelText, Qt::ElideRight, 410));
}

void RadioNowPlayingWindow::onGetPosition(int position, QString)
{
    ui->currentPositionLabel->setText(time_mmss(position));
    if (ui->streamLengthLabel->text() != "--:--") {
        if (!ui->positionSlider->isSliderDown())
            ui->positionSlider->setValue(position);
    } else {
        if (ui->positionSlider->value() != 0)
            ui->positionSlider->setValue(0);
    }
}

void RadioNowPlayingWindow::onBufferingInfo(float status)
{
    if (status == 1.0) {
        ui->bufferBar->hide();
        ui->seekWidget->show();
        ui->bufferBar->setRange(0, 0);
    } else { // status != 1.0
        int percentage = (int)(status*100);
        ui->bufferBar->setRange(0, 100);
        ui->bufferBar->setValue(percentage);
        ui->bufferBar->setFormat(tr("Buffering") + " %p%");

        if (ui->bufferBar->isHidden()) {
            ui->seekWidget->hide();
            ui->bufferBar->show();
        }
    }
}

void RadioNowPlayingWindow::onNextButtonClicked()
{
    if (ui->nextButton->isDown()) {
        buttonWasDown = true;
        mafwrenderer->setPosition(SeekRelative, 3);
        mafwrenderer->getPosition();
    } else {
        if (!buttonWasDown)
            mafwrenderer->next();
        buttonWasDown = false;
    }
}

void RadioNowPlayingWindow::onPreviousButtonClicked()
{
    if (ui->prevButton->isDown()) {
        buttonWasDown = true;
        mafwrenderer->setPosition(SeekRelative, -3);
        mafwrenderer->getPosition();
    } else {
        if (!buttonWasDown)
            mafwrenderer->previous();
        buttonWasDown = false;
    }
}

void RadioNowPlayingWindow::onPositionSliderPressed()
{
    onPositionSliderMoved(ui->positionSlider->value());
}

void RadioNowPlayingWindow::onPositionSliderReleased()
{
    ui->currentPositionLabel->setText(time_mmss(ui->positionSlider->value()));
    mafwrenderer->setPosition(SeekAbsolute, ui->positionSlider->value());
}

void RadioNowPlayingWindow::onPositionSliderMoved(int position)
{
    ui->currentPositionLabel->setText(time_mmss(position));
    if (!lazySliders)
        mafwrenderer->setPosition(SeekAbsolute, position);
}

void RadioNowPlayingWindow::onRendererMetadataRequested(GHashTable *metadata, QString, QString error)
{
    if (metadata != NULL) {
        QString station;
        QString albumArt;
        bool isSeekable;
        int duration;
        GValue *v;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string(v)) : QString();

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ARTIST);
        artist = v ? QString::fromUtf8(g_value_get_string(v)) : QString();

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int64 (v) : Duration::Unknown;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_IS_SEEKABLE);
        isSeekable = v ? g_value_get_boolean (v) : false;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_RENDERER_ART_URI); // it's really a path, not a URI
        setAlbumImage(v ? QString::fromUtf8(g_value_get_string(v)) : defaultRadioImage);

        updateSongLabel();

        streamIsSeekable(isSeekable);

        if (duration != Duration::Unknown) {
            ui->streamLengthLabel->setText(time_mmss(duration));
            ui->positionSlider->setRange(0, duration);
        } else {
            ui->streamLengthLabel->setText("--:--");
        }
    }

    if (!error.isEmpty())
        qDebug() << error;
}

void RadioNowPlayingWindow::onSourceMetadataRequested(QString objectId, GHashTable *metadata, QString error)
{
    if (objectId != currentObjectId) return;

    if (metadata != NULL) {
        QString station;
        GValue *v;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        station = QString::fromUtf8(g_value_get_string (v));
        ui->stationLabel->setText(QFontMetrics(ui->stationLabel->font()).elidedText(station, Qt::ElideRight, 410));

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_URI);
        uri = v ? QString::fromUtf8(g_value_get_string(v)) : "";
    }

    if (!error.isEmpty())
        qDebug() << error;
}
#endif

void RadioNowPlayingWindow::setAlbumImage(QString image)
{
    qDeleteAll(albumArtScene->items());

    ui->albumArtView->setScene(albumArtScene);
    albumArtScene->setBackgroundBrush(QBrush(Qt::transparent));

    mirror *m = new mirror();
    albumArtScene->addItem(m);

    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(QPixmap(image).scaled(QSize(295, 295),
                                                        Qt::IgnoreAspectRatio,
                                                        Qt::SmoothTransformation));

    albumArtScene->addItem(item);
    m->setItem(item);
}

void RadioNowPlayingWindow::orientationChanged(int w, int h)
{
    if (w > h) { // Landscape
        ui->verticalLayout->setContentsMargins(-1, 5, -1, -1);
        ui->mainLayout->setDirection(QBoxLayout::LeftToRight);
        ui->volumeWidget->show();
        ui->spacerWidget->show();
        ui->spacerWidget2->show();
        ui->metadataWidget->setFixedWidth(440);
        ui->stationLabel->setAlignment(Qt::AlignLeft);
        ui->songLabel->setAlignment(Qt::AlignLeft);
    } else { // Portrait
        ui->volumeWidget->hide();
        ui->mainLayout->setDirection(QBoxLayout::TopToBottom);
        ui->verticalLayout->setContentsMargins(-1, 80, -1, -1);
        ui->spacerWidget2->hide();
        ui->spacerWidget->hide();
        ui->metadataWidget->setFixedWidth(470);
        ui->stationLabel->setAlignment(Qt::AlignHCenter);
        ui->songLabel->setAlignment(Qt::AlignHCenter);
    }
}

#ifdef Q_WS_MAEMO_5
void RadioNowPlayingWindow::showFMTXDialog()
{
    FMTXDialog *fmtxDialog = new FMTXDialog(this);
    fmtxDialog->show();
}
#endif

void RadioNowPlayingWindow::showBookmarkDialog()
{
#ifdef MAFW
    BookmarkDialog(this, mafwFactory, Media::Audio, uri, ui->stationLabel->text()).exec();
#endif
}

void RadioNowPlayingWindow::onNextButtonPressed()
{
    if (ui->nextButton->isDown())
        ui->nextButton->setIcon(QIcon(nextButtonPressedIcon));
    else
        ui->nextButton->setIcon(QIcon(nextButtonIcon));
}

void RadioNowPlayingWindow::onPrevButtonPressed()
{
    if (ui->prevButton->isDown())
        ui->prevButton->setIcon(QIcon(prevButtonPressedIcon));
    else
        ui->prevButton->setIcon(QIcon(prevButtonIcon));
}

void RadioNowPlayingWindow::onStopButtonPressed()
{
    if (ui->playButton->isDown())
        ui->playButton->setIcon(QIcon(stopButtonPressedIcon));
    else
        ui->playButton->setIcon(QIcon(stopButtonIcon));
}

void RadioNowPlayingWindow::streamIsSeekable(bool seekable)
{
#ifdef MAFW
    if (mafwState == Playing) {
        if (seekable) {
            onStateChanged(Playing);
        } else {
            ui->playButton->setIcon(QIcon(stopButtonIcon));
            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
            connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(stop()));
            connect(ui->playButton, SIGNAL(pressed()), this, SLOT(onStopButtonPressed()));
            connect(ui->playButton, SIGNAL(released()), this, SLOT(onStopButtonPressed()));
            mafwrenderer->getPosition();
            if (!positionTimer->isActive())
                positionTimer->start();
        }
    }
#endif
}

void RadioNowPlayingWindow::togglePlayback()
{
#ifdef MAFW
    if (mafwState == Playing)
        mafwrenderer->pause();
    else if (mafwState == Paused)
        mafwrenderer->resume();
    else if (mafwState == Stopped)
        mafwrenderer->play();
#endif
}

#ifdef MAFW
void RadioNowPlayingWindow::onPlayMenuRequested(const QPoint &pos)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Stop playback"), mafwrenderer, SLOT(stop()));
    contextMenu->exec(this->mapToGlobal(pos));
}
#endif
