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
