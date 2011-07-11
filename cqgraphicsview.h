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
    bool isPressed();

signals:
    void clicked();
    void pressed();
    void released();

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

public slots:

private:
    bool isPosOnItem(const QPoint &p);
    bool m_pressed;
};

#endif // CQGRAPHICSVIEW_H
