#ifndef PLAYLISTPICKER_H
#define PLAYLISTPICKER_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QKeyEvent>

#include "ui_playlistpicker.h"
#include "includes.h"
#include "confirmdialog.h"
#include "rotator.h"

#include "mafw/mafwplaylistadapter.h"
#include "mafw/mafwplaylistmanageradapter.h"

namespace Ui {
    class PlaylistPicker;
}

class PlaylistPicker : public QDialog
{
    Q_OBJECT

public:
    explicit PlaylistPicker(QWidget *parent = 0);
    ~PlaylistPicker();

    QString playlistName;

private:
    Ui::PlaylistPicker *ui;

    void keyPressEvent(QKeyEvent *e);

    QDialog *createPlaylistDialog;
    QLineEdit *playlistNameEdit;

private slots:
    void onCreatePlaylist();
    void onCreatePlaylistAccepted();
    void onItemActivated(QListWidgetItem *item);
    void onOrientationChanged(int h, int w);
};

#endif // PLAYLISTPICKER_H
