#include "videonowplayingwindow.h"

VideoNowPlayingWindow::VideoNowPlayingWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideoNowPlayingWindow)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    setAttribute(Qt::WA_DeleteOnClose);
    volumeTimer = new QTimer(this);
    volumeTimer->setInterval(3000);
    this->setIcons();
    this->orientationChanged();
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
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
#ifdef Q_WS_MAEMO_5
    QDBusConnection::sessionBus().connect("com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer",
                                          "/com/nokia/mafw/renderer/gstrenderer",
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

#ifdef Q_WS_MAEMO_5
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
    if(screenGeometry.width() > screenGeometry.height()) {
        ui->deleteButton->show();
        ui->shareButton->show();
        ui->volumeButton->show();
    } else {
        ui->deleteButton->hide();
        ui->shareButton->hide();
        ui->volumeButton->hide();
        if(!ui->volumeSlider->isHidden())
            ui->volumeSlider->hide();
    }
}
