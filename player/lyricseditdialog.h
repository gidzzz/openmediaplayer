#ifndef LYRICSEDITDIALOG_H
#define LYRICSEDITDIALOG_H

#include <QMainWindow>

#include <QShortcut>

#include "ui_lyricseditdialog.h"
#include "missioncontrol.h"
#include "texteditautoresizer.h"

namespace Ui {
    class LyricsEditDialog;
}

class LyricsEditDialog : public QMainWindow
{
    Q_OBJECT

public:
    explicit LyricsEditDialog(QString artist, QString title, QWidget *parent = 0);
    ~LyricsEditDialog();

private:
    Ui::LyricsEditDialog *ui;

    QString artist;
    QString title;

private slots:
    void save();
};

#endif // LYRICSEDITDIALOG_H
