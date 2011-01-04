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
    timer = new QTimer(this);
    timer->setInterval(100);
    this->stopAnimation();
    mafwrenderer = new MafwRendererAdapter();
    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    connect(timer, SIGNAL(timeout()), this, SLOT(startAnimation()));
    frame = 1;
}

NowPlayingIndicator::~NowPlayingIndicator()
{
    delete ui;
}

void NowPlayingIndicator::setBackgroundImage(const QString &image)
{
    this->indicatorImage = image;
    this->update();
}

void NowPlayingIndicator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(this->rect(), QImage(this->indicatorImage));
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
    QString frameImage = "/usr/share/icons/hicolor/scalable/hildon/mediaplayer_nowplaying_indicator" + QString::number(frame) + ".png";
    this->setBackgroundImage(frameImage);
}

void NowPlayingIndicator::stopAnimation()
{
    if(timer->isActive())
        timer->stop();
    this->setBackgroundImage(idleFrame);
}

void NowPlayingIndicator::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        emit clicked();
}
