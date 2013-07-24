#ifndef CQGRAPHICSVIEW_H
#define CQGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>

class CQGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CQGraphicsView(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
};

#endif // CQGRAPHICSVIEW_H
