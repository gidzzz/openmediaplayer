#ifndef EDITLYRICS_H
#define EDITLYRICS_H

#include <QMainWindow>

#include "ui_editlyrics.h"
#include "nowplayingwindow.h"
#include "texteditautoresizer.h"

namespace Ui {
    class EditLyrics;
}

class EditLyrics : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditLyrics(QString artist, QString title, QWidget *parent = 0);
    ~EditLyrics();

private:
    Ui::EditLyrics *ui;

    QString artist;
    QString title;

private slots:
    void save();
};

#endif // EDITLYRICS_H
