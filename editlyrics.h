#ifndef EDITLYRICS_H
#define EDITLYRICS_H

#include <QMainWindow>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

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
    explicit EditLyrics(QWidget *parent = 0, QString lyricsFile = "",
                        QString artist = "", QString title = "");
    ~EditLyrics();
    QString file;
    QString lyrics;

private:
    Ui::EditLyrics *ui;

private slots:
    void on_pushButton_pressed();
};

#endif // EDITLYRICS_H
