#include "nowplayingindicator.h"

NowPlayingIndicator::NowPlayingIndicator(QWidget *parent) :
    QWidget(parent),
#ifdef Q_WS_MAEMO_5
    ui(new Ui::NowPlayingIndicator)
#else
    ui(new Ui::NowPlayingIndicator)
#endif
{
    ui->setupUi(this);
    setAttribute(Qt::WA_OpaquePaintEvent);
    timer = new QTimer(this);
    timer->setInterval(100);
    this->stopAnimation();
    mafwrenderer = new MafwRendererAdapter();
    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    connect(timer, SIGNAL(timeout()), this, SLOT(startAnimation()));
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

void NowPlayingIndicator::onStateChanged(int state)
{
    if(state == Paused)
        this->stopAnimation();
    else
        timer->start();
}

void NowPlayingIndicator::startAnimation()
{
    // Update the icon
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

void NowPlayingIndicator::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        emit clicked();
}
