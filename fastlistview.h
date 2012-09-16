#ifndef FASTLISTVIEW_H
#define FASTLISTVIEW_H

#include <QListView>
#include <QKeyEvent>

class FastListView : public QListView
{
    Q_OBJECT

public:
    FastListView(QWidget *parent) : QListView(parent) { };

protected:
    void keyPressEvent(QKeyEvent *e)
    {
        switch (e->key()) {
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Enter:
                QListView::keyPressEvent(e); break;
            default:
                e->ignore();
        }
    }
};

#endif // FASTLISTVIEW_H
