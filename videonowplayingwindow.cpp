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
#ifdef Q_WS_MAEMO_5
#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#endif

VideoNowPlayingWindow::VideoNowPlayingWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideoNowPlayingWindow)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if(screenGeometry.width() > screenGeometry.height()) {
        portrait = false;
        this->orientationChanged();
        this->onLandscapeMode();
    } else {
        portrait = true;
    }
    rotator = new QMaemo5Rotator(QMaemo5Rotator::AutomaticBehavior);
    //http://www.gossamer-threads.com/lists/maemo/developers/54239
    quint32 disable = {0};
    Atom winPortraitModeSupportAtom = XInternAtom(QX11Info::display(), "_HILDON_PORTRAIT_MODE_SUPPORT", false);
    XChangeProperty(QX11Info::display(), winId(), winPortraitModeSupportAtom, XA_CARDINAL, 32, PropModeReplace, (uchar*) &disable, 1);
    if(portrait) {
        QTimer::singleShot(1000, this, SLOT(onPortraitMode()));
    }
    this->setDNDAtom(true);
#endif
    setAttribute(Qt::WA_DeleteOnClose);
    volumeTimer = new QTimer(this);
    volumeTimer->setInterval(3000);
    this->setIcons();
    this->connectSignals();
    ui->volumeSlider->hide();
}

VideoNowPlayingWindow::~VideoNowPlayingWindow()
{
    delete ui;
}

void VideoNowPlayingWindow::setIcons()
{
    ui->wmCloseButton->setIcon(QIcon(wmCloseIcon));
    ui->prevButton->setIcon(QIcon(prevButtonIcon));
    ui->playButton->setIcon(QIcon(playButtonIcon));
    ui->nextButton->setIcon(QIcon(nextButtonIcon));
    ui->deleteButton->setIcon(QIcon(deleteButtonIcon));
    ui->shareButton->setIcon(QIcon(shareButtonIcon));
    ui->volumeButton->setIcon(QIcon(volumeButtonIcon));
}

void VideoNowPlayingWindow::connectSignals()
{
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(volumeWatcher()));
    connect(volumeTimer, SIGNAL(timeout()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeSlider, SIGNAL(sliderPressed()), volumeTimer, SLOT(stop()));
    connect(ui->volumeSlider, SIGNAL(sliderReleased()), volumeTimer, SLOT(start()));
    //connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
#ifdef Q_WS_MAEMO_5
    connect(rotator, SIGNAL(portrait()), this, SLOT(onPortraitMode()));
    connect(rotator, SIGNAL(landscape()), this, SLOT(onLandscapeMode()));
    QDBusConnection::sessionBus().connect("com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer",
                                          "/700com/nokia/mafw/renderer/gstrenderer",
                                          "com.nokia.mafw.extension",
                                          "property_changed",
                                          this, SLOT(onVolumeChanged(const QDBusMessage &)));
#endif
}

void VideoNowPlayingWindow::toggleVolumeSlider()
{
    if(ui->volumeSlider->isHidden()) {
        ui->buttonWidget->hide();
        ui->volumeSlider->show();
    } else {
        ui->volumeSlider->hide();
        ui->buttonWidget->show();
        if(volumeTimer->isActive())
            volumeTimer->stop();
    }
}

#ifdef MAFW
void VideoNowPlayingWindow::onVolumeChanged(const QDBusMessage &msg)
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

void VideoNowPlayingWindow::volumeWatcher()
{
    if(!ui->volumeSlider->isHidden())
        volumeTimer->start();
}

void VideoNowPlayingWindow::orientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->controlOverlay->setGeometry((screenGeometry.width() / 2)-(ui->controlOverlay->width()/2),
                                    (screenGeometry.height() / 2)-(ui->controlOverlay->height()/2),
                                    ui->controlOverlay->width(), ui->controlOverlay->height());
    ui->toolbarOverlay->setGeometry(0, screenGeometry.height()-ui->toolbarOverlay->height(),
                                    screenGeometry.width(), ui->toolbarOverlay->height());
    ui->wmCloseButton->setGeometry(screenGeometry.width()-ui->wmCloseButton->width(), 0,
                                   ui->wmCloseButton->width(), ui->wmCloseButton->height());
}

#ifdef Q_WS_MAEMO_5
void VideoNowPlayingWindow::onPortraitMode()
{
    ui->wmCloseButton->setGeometry(0, 0, 56, 112);
    ui->wmCloseButton->setIconSize(QSize(56, 112));
    QTransform t;
    t = t.rotate(-90, Qt::ZAxis);
    ui->wmCloseButton->setIcon(QIcon(QPixmap(wmCloseIcon).transformed(t)));
    ui->prevButton->setIcon(QIcon(QPixmap(prevButtonIcon).transformed(t)));
    ui->playButton->setIcon(QIcon(QPixmap(playButtonIcon).transformed(t)));
    ui->nextButton->setIcon(QIcon(QPixmap(nextButtonIcon).transformed(t)));
    ui->deleteButton->setIcon(QIcon(QPixmap(deleteButtonIcon).transformed(t)));
    ui->shareButton->setIcon(QIcon(QPixmap(shareButtonIcon).transformed(t)));
    ui->volumeButton->setIcon(QIcon(QPixmap(volumeButtonIcon).transformed(t)));
    ui->controlLayout->setDirection(QBoxLayout::BottomToTop);
    ui->controlOverlay->setGeometry(360, 70, 101, 318);
    if(!ui->toolbarOverlay->isHidden())
        ui->toolbarOverlay->hide();
    if(ui->portraittoolBar->isHidden())
        ui->portraittoolBar->show();
    ui->portraittoolBar->update();
}

void VideoNowPlayingWindow::onLandscapeMode()
{
    this->setIcons();
    ui->deleteButton->show();
    ui->shareButton->show();
    ui->volumeButton->show();
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->wmCloseButton->setIconSize(QSize(112, 56));
    ui->wmCloseButton->setGeometry(screenGeometry.width()-112, 0, 112, 56);
    ui->controlLayout->setDirection(QBoxLayout::LeftToRight);
    ui->controlOverlay->setGeometry(230, 170, 318, 114);
    if(ui->toolbarOverlay->isHidden())
        ui->toolbarOverlay->show();
    if(!ui->portraittoolBar->isHidden())
        ui->portraittoolBar->hide();
}

void VideoNowPlayingWindow::setDNDAtom(bool dnd)
{
    quint32 enable;
    if (dnd)
        enable = 1;
    else
        enable = 0;
    Atom winDNDAtom = XInternAtom(QX11Info::display(), "_HILDON_DO_NOT_DISTURB", false);
    XChangeProperty(QX11Info::display(), winId(), winDNDAtom, XA_INTEGER, 32, PropModeReplace, (uchar*) &enable, 1);
}

#endif
