#include "qrotatedlabel.h"
#include <QDebug>

QRotatedLabel::QRotatedLabel(QWidget *parent) :
    QWidget(parent)
{
}

void QRotatedLabel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QTransform t;
    t = t.rotate(-90, Qt::ZAxis);
    painter.setTransform(t);
    if(this->text.isEmpty())
        this->text = "00:00";
    painter.drawText(-this->height(), this->width()/2, this->text);
}

void QRotatedLabel::setText(QString labelText)
{
    this->text = labelText;
    this->update();
}
