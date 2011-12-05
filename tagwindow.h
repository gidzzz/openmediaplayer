#ifndef TAGWINDOW_H
#define TAGWINDOW_H

#include <QDialog>
#include <QString>

#include "ui_tagwindow.h"
//#include "mafw/mafwsourceadapter.h"
//#include "mafw/mafwadapterfactory.h"
//#include "nowplayingwindow.h"
//#include "glib-2.0/glib/ghash.h"

namespace Ui {
    class TagWindow;
}

class TagWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TagWindow(QWidget *parent = 0, QString id = "", QString title = "", QString artist = "", QString album = "");
    ~TagWindow();

    QString objectId;
    QString title;
    QString artist;
    QString album;

private:
    Ui::TagWindow *ui;

private slots:
    void on_saveButton_pressed();

};

#endif // TAGWINDOW_H
