#ifndef CQGRAPHICSVIEW_H
#define CQGRAPHICSVIEW_H

#include <QObject>
#include <QWidget>
#include <QMouseEvent>
#include <QGraphicsView>

class CQGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CQGraphicsView(QWidget *parent = 0);
    ~CQGraphicsView();

signals:
    void clicked();
protected:
    void mouseReleaseEvent(QMouseEvent *event);

public slots:

};

#endif // CQGRAPHICSVIEW_H
