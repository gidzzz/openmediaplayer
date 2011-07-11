#ifndef EDITLYRICS_H
#define EDITLYRICS_H

#include <QMainWindow>

namespace Ui {
    class EditLyrics;
}

class EditLyrics : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditLyrics(QWidget *parent = 0, QString d1 = "",
                        QString d2 = "", QString d3 = "");
    ~EditLyrics();
    QString file;
    QString lyrics;
    int modified;

private:
    Ui::EditLyrics *ui;

private slots:
    void on_pushButton_pressed();
};

#endif // EDITLYRICS_H
