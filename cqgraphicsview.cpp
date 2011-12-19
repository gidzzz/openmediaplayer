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

#include "cqgraphicsview.h"

CQGraphicsView::CQGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
}

CQGraphicsView::~CQGraphicsView()
{
}

void CQGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
    } else {
        m_pressed = false;
    }
    emit pressed();
}

void CQGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && this->isPosOnItem(event->pos()))
        emit clicked();
    m_pressed = false;
    emit released();
}

bool CQGraphicsView::isPosOnItem(const QPoint &p)
{
    return rect().contains(p);
}

bool CQGraphicsView::isPressed()
{
    return m_pressed;
}
