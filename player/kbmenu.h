#ifndef KBMENU_H
#define KBMENU_H

#include <QMenu>
#include <QKeyEvent>

class KbMenu : public QMenu
{
    Q_OBJECT

public:
    KbMenu(QWidget *parent = 0) : QMenu(parent) { }

protected:
    void keyPressEvent(QKeyEvent *e)
    {
        if (e->key() == Qt::Key_Backspace)
            this->close();
        else
            QMenu::keyPressEvent(e);
    }
};

#endif // KBMENU_H
