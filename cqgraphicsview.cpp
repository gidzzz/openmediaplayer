#include "cqgraphicsview.h"

CQGraphicsView::CQGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
}

CQGraphicsView::~CQGraphicsView()
{
}

void CQGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        emit clicked();
}
