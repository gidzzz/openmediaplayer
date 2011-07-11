#ifndef TAGWINDOW_H
#define TAGWINDOW_H

#include <QDialog>
#include <QString>

namespace Ui {
    class TagWindow;
}

class TagWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TagWindow(QWidget *parent = 0, QString d1 = "",
                       QString d2 = "", QString d3 = "", QString d4 = "");
    ~TagWindow();
    QString id;
    QString artist, album, title;

private:
    Ui::TagWindow *ui;

private slots:
    void on_pushButton_pressed();

};

#endif // TAGWINDOW_H
