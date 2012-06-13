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
#include "ui_radionowplayingwindow.h"

RadioNowPlayingWindow::RadioNowPlayingWindow(QWidget *parent, MafwAdapterFactory *factory) :
    QMainWindow(parent),
    ui(new Ui::RadioNowPlayingWindow)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwRadioSource(factory->getRadioSource()),
    playlist(factory->getPlaylistAdapter())
#endif
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

#ifdef Q_WS_MAEMO_5
    QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");
#else
    QColor secondaryColor(156, 154, 156);
#endif
    ui->songLabel->setStyleSheet(QString("color: rgb(%1, %2, %3);")
                                 .arg(secondaryColor.red())
                                 .arg(secondaryColor.green())
                                 .arg(secondaryColor.blue()));
    this->updateSongLabel();

    ui->volumeSlider->hide();
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    this->setIcons();
    volumeTimer = new QTimer(this);
    volumeTimer->setInterval(3000);

    positionTimer = new QTimer(this);
    positionTimer->setInterval(1000);

    ui->bufferBar->setRange(0, 2);
    ui->bufferBar->setValue(1);

    ui->bufferBar->hide();

    this->connectSignals();

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    orientationChanged(rotator->width(), rotator->height());

#ifdef MAFW
    mafwrenderer->getStatus();
    mafwrenderer->getPosition();
    mafwrenderer->getVolume();
    mafwrenderer->getCurrentMetadata();
#endif
}

RadioNowPlayingWindow::~RadioNowPlayingWindow()
{
    delete ui;
}

void RadioNowPlayingWindow::connectSignals()
{
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(volumeWatcher()));
    connect(volumeTimer, SIGNAL(timeout()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeSlider, SIGNAL(sliderPressed()), this, SLOT(onVolumeSliderPressed()));
    connect(ui->volumeSlider, SIGNAL(sliderReleased()), this, SLOT(onVolumeSliderReleased()));
#ifdef Q_WS_MAEMO_5
    connect(ui->actionFM_transmitter, SIGNAL(triggered()), this, SLOT(showFMTXDialog()));
#endif
    connect(ui->nextButton, SIGNAL(pressed()), this, SLOT(onNextButtonPressed()));
    connect(ui->nextButton, SIGNAL(released()), this, SLOT(onNextButtonPressed()));
    connect(ui->prevButton, SIGNAL(pressed()), this, SLOT(onPrevButtonPressed()));
    connect(ui->prevButton, SIGNAL(released()), this, SLOT(onPrevButtonPressed()));
#ifdef MAFW
    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    connect(mafwrenderer, SIGNAL(mediaChanged(int,char*)), this, SLOT(onMediaChanged(int,char*)));
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));
    connect(mafwrenderer, SIGNAL(signalGetVolume(int)), ui->volumeSlider, SLOT(setValue(int)));
    connect(ui->volumeSlider, SIGNAL(sliderMoved(int)), mafwrenderer, SLOT(setVolume(int)));
    connect(mafwrenderer, SIGNAL(signalGetPosition(int,QString)), this, SLOT(onGetPosition(int,QString)));
    connect(mafwrenderer, SIGNAL(bufferingInfo(float)), this, SLOT(onBufferingInfo(float)));
    connect(mafwrenderer, SIGNAL(metadataChanged(QString,QVariant)), this, SLOT(onRendererMetadataChanged(QString,QVariant)));
    connect(mafwrenderer, SIGNAL(signalGetCurrentMetadata(GHashTable*,QString,QString)),
            this, SLOT(onRendererMetadataRequested(GHashTable*,QString,QString)));
    connect(mafwRadioSource, SIGNAL(signalMetadataResult(QString,GHashTable*,QString)),
            this, SLOT(onSourceMetadataRequested(QString,GHashTable*,QString)));
    connect(positionTimer, SIGNAL(timeout()), mafwrenderer, SLOT(getPosition()));
    connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(onNextButtonClicked()));
    connect(ui->prevButton, SIGNAL(clicked()), this, SLOT(onPreviousButtonClicked()));

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
    this->mafwState = state;

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
    else if (state == Transitioning) {
        ui->songProgress->setEnabled(false);
        ui->songProgress->setValue(0);
        ui->songProgress->setRange(0, 99);
        ui->currentPositionLabel->setText("00:00");
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
    ui->songLabel->setText(tr("(unknown artist)") + " / " + tr("(unknown song)"));

    if (objectId)
        mafwRadioSource->getMetadata(objectId, MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE));
    else
        ui->stationLabel->setText(tr("(unknown station)"));
}

void RadioNowPlayingWindow::onRendererMetadataChanged(QString name, QVariant value)
{
    qDebug() << "Metadata changed:" << name << "=" << value;

    mafwrenderer->getCurrentMetadata();

    if (name == "is-seekable" /*MAFW_METADATA_KEY_IS_SEEKABLE*/) {
        ui->songProgress->setEnabled(value.toBool());
        this->streamIsSeekable(value.toBool());
    }
    else if (name == "artist") {
        this->artist = value.toString();
        this->updateSongLabel();
    }
    else if (name == "title") {
        this->title = value.toString();
        this->updateSongLabel();
    }
    else if (name == "duration" /*MAFW_METADATA_KEY_DURATION*/) {
        ui->streamLengthLabel->setText(time_mmss(value.toInt()));
    }
    else if (name == "renderer-art-uri") {
        ui->albumArt->setPixmap(QPixmap(value.toString()));
    }
}

void RadioNowPlayingWindow::onGetStatus(MafwPlaylist*, uint, MafwPlayState state, const char *, QString)
{
    this->onStateChanged(state);
}

void RadioNowPlayingWindow::updateSongLabel()
{
    QString labelText = this->title;

    if (title.isEmpty())
        labelText = tr("(unknown artist)") + " / " + tr("(unknown song)");
    else if (!artist.isEmpty())
        labelText.prepend(artist + " / ");

    ui->songLabel->setText(labelText);
}

void RadioNowPlayingWindow::onGetPosition(int position, QString)
{
    ui->currentPositionLabel->setText(time_mmss(position));
    if (ui->streamLengthLabel->text() != "--:--")
        ui->songProgress->setValue(position);
    else {
        if (ui->songProgress->value() != 0)
            ui->songProgress->setValue(0);
    }
}

void RadioNowPlayingWindow::onBufferingInfo(float buffer)
{
    if (buffer != 0.0) {
        int percentage = (int)(buffer*100);
        ui->bufferBar->setRange(0, 100);
        ui->bufferBar->setValue(percentage);

        ui->bufferBar->setFormat(tr("Buffering") + " %p%");

        if (buffer == 1.0) {
            ui->bufferBar->hide();
            ui->seekWidget->show();
            if (!positionTimer->isActive())
                positionTimer->start();
        }
    } else { // buffer == 0.0
        ui->bufferBar->setRange(0, 0);
        ui->bufferBar->setValue(-1);

        // Qt doesn't want to display the label in the bouncing mode
        /*ui->bufferBar->setFormat(tr("Connecting"));*/

        if (positionTimer->isActive())
            positionTimer->stop();
    }

    if (buffer != 1.0 && !ui->bufferBar->isVisible()) {
        ui->seekWidget->hide();
        ui->bufferBar->show();
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

void RadioNowPlayingWindow::onRendererMetadataRequested(GHashTable *metadata, QString, QString error)
{
    if (metadata != NULL) {
        QString station;
        QString albumArt;
        bool isSeekable;
        int duration;
        GValue *v;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        this->title = v ? QString::fromUtf8(g_value_get_string (v)) : "";

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ARTIST);
        this->artist = v ? QString::fromUtf8(g_value_get_string (v)) : "";

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ORGANIZATION);
        station = v ? QString::fromUtf8(g_value_get_string (v)) : "";

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int (v) : Duration::Unknown;
        this->streamDuration = duration;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_IS_SEEKABLE);
        isSeekable = v ? g_value_get_boolean (v) : false;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_URI);
        if (v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if (file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                ui->albumArt->setPixmap(QPixmap(QString::fromUtf8(filename)));
            }
        } else {
            ui->albumArt->setPixmap(QPixmap(radioImage));
        }

        this->updateSongLabel();

        if (!station.isEmpty())
            ui->stationLabel->setText(station);

        if (duration != Duration::Unknown) {
            ui->streamLengthLabel->setText(time_mmss(duration));
            ui->songProgress->setRange(0, duration);
        } else
            ui->streamLengthLabel->setText("--:--");

        this->streamIsSeekable(isSeekable);
    }

    if (!error.isEmpty())
        qDebug() << error;
}

void RadioNowPlayingWindow::onSourceMetadataRequested(QString, GHashTable *metadata, QString error)
{
    if (metadata != NULL) {
        QString station;
        GValue *v;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        station = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown station)");

        ui->stationLabel->setText(station);
    }

    if (!error.isEmpty())
        qDebug() << error;
}
#endif

void RadioNowPlayingWindow::orientationChanged(int w, int h)
{
    if (w > h) { // Landscape
        ui->mainLayout->setDirection(QBoxLayout::LeftToRight);
        ui->volumeWidget->show();
        ui->spacerWidget->show();
        ui->spacerWidget2->show();
        ui->metadataWidget->setFixedWidth(440);
    } else { // Portrait
        ui->volumeWidget->hide();
        ui->mainLayout->setDirection(QBoxLayout::TopToBottom);
        ui->spacerWidget2->hide();
        ui->spacerWidget->hide();
        ui->metadataWidget->setFixedWidth(470);
    }
}

#ifdef Q_WS_MAEMO_5
void RadioNowPlayingWindow::showFMTXDialog()
{
    FMTXDialog *fmtxDialog = new FMTXDialog(this);
    fmtxDialog->show();
}
#endif

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
    if (seekable) {
        if (this->mafwState == Playing) {
            this->onStateChanged(Playing);
        }
    } else {
        if (this->mafwState == Playing) {
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
    ui->songProgress->setEnabled(seekable);
#endif
}
