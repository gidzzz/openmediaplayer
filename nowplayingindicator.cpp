#include "nowplayingindicator.h"

NowPlayingIndicator::NowPlayingIndicator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NowPlayingIndicator)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_OpaquePaintEvent);
    timer = new QTimer(this);
    timer->setInterval(100);
    this->stopAnimation();
#ifdef Q_WS_MAEMO_5
    mafwrenderer = new MafwRendererAdapter();
    this->mafwState = Paused;
    deviceEvents = new Maemo5DeviceEvents(this);
#endif
    this->connectSignals();
    frame = 0;
    images << QPixmap(idleFrame);
    for (int i = 1; i < 12; i++)
        images << QPixmap("/usr/share/icons/hicolor/scalable/hildon/mediaplayer_nowplaying_indicator" + QString::number(i) + ".png");
}

NowPlayingIndicator::~NowPlayingIndicator()
{
    delete ui;
}

void NowPlayingIndicator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, images[frame]);
}

void NowPlayingIndicator::connectSignals()
{
#ifdef Q_WS_MAEMO_5
    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    connect(deviceEvents, SIGNAL(screenLocked(bool)), this, SLOT(onTkLockChanged(bool)));
#endif
    connect(timer, SIGNAL(timeout()), this, SLOT(startAnimation()));
}

#ifdef Q_WS_MAEMO_5
void NowPlayingIndicator::onStateChanged(int state)
{
    this->mafwState = state;
    if(state == Paused || state == Stopped)
        this->stopAnimation();
    else
        timer->start();
}
#endif
#ifdef Q_WS_MAEMO_5
void NowPlayingIndicator::onTkLockChanged(bool state)
{
    if(state) {
#ifdef DEBUG
        qDebug() << "NowPlayingIndicator: Screen locked, stopping animation";
#endif
        this->stopAnimation();
    } else {
#ifdef DEBUG
        qDebug() << "NowPlayingIndicator: Screen unlocked, starting animation";
#endif
        if(!deviceEvents->isScreenLocked() && this->mafwState == Playing) {
            timer->start();
        } else {
#ifdef DEBUG
            qDebug() << "NowPlayingIndicator: Screen locked, animation blocked.";
#endif
            if(timer->isActive())
                timer->stop();
        }
    }
}
#endif

void NowPlayingIndicator::startAnimation()
{
    // Update the icon only if the window is active
        if(frame == 11)
            frame = 1;
        else
            frame++;
        this->update();
}

void NowPlayingIndicator::stopAnimation()
{
    if(timer->isActive())
        timer->stop();
    this->frame = 0;
    this->repaint();
}

void NowPlayingIndicator::mouseReleaseEvent(QMouseEvent *)
{
    //if(event->button() == Qt::LeftButton)
     //   emit clicked();
    // This button does some weird behavior, it always creates a new NowPlayingWindow...
    // If video was running, show its window.
    // If x was running, shows its window.
    // TODO: Update code as mafw is integrated.
#ifdef Q_WS_MAEMO_5
    NowPlayingWindow *songs = new NowPlayingWindow(this, mafwrenderer);
#else
    NowPlayingWindow *songs = new NowPlayingWindow(this, 0);
#endif
    songs->setAttribute(Qt::WA_DeleteOnClose);
    songs->show();
}

void NowPlayingIndicator::hideEvent(QHideEvent *)
{
    this->stopAnimation();
}

void NowPlayingIndicator::showEvent(QShowEvent *)
{
    if(!deviceEvents->isScreenLocked() && this->mafwState == Playing)
        timer->start();
}
