#ifndef LYRICSSEARCHDIALOG_H
#define LYRICSSEARCHDIALOG_H

#include <QDialog>
#include <QPushButton>

#include "ui_lyricssearchdialog.h"
#include "includes.h"
#include "rotator.h"
#include "missioncontrol.h"

namespace Ui {
    class LyricsSearchDialog;
}

class LyricsSearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LyricsSearchDialog(QWidget *parent = 0);
    ~LyricsSearchDialog();

private:
    Ui::LyricsSearchDialog *ui;

    QString artist;
    QString title;

private slots:
    void onOrientationChanged(int w, int h);
    void onMetadataChanged(QString key, QVariant value);
    void accept();
};

#endif // LYRICSSEARCHDIALOG_H
