#include "cqgraphicsview.h"

CQGraphicsView::CQGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
}

void CQGraphicsView::mousePressEvent(QMouseEvent *e)
{
    e->ignore();
}

void CQGraphicsView::mouseReleaseEvent(QMouseEvent *e)
{
    e->ignore();
}

void CQGraphicsView::mouseDoubleClickEvent(QMouseEvent *e)
{
    e->ignore();
}
