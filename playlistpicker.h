#ifndef PLAYLISTPICKER_H
#define PLAYLISTPICKER_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QLineEdit>
#include <QListWidget>

#include "includes.h"
#include "rotator.h"

#ifdef MAFW
    #include "mafw/mafwplaylistadapter.h"
    #include "mafw/mafwplaylistmanageradapter.h"
#endif

namespace Ui {
    class PlaylistPicker;
}

class PlaylistPicker : public QDialog
{
    Q_OBJECT

public:
    explicit PlaylistPicker(QWidget *parent = 0);
    ~PlaylistPicker();
    MafwPlaylist *playlist;
    QString playlistName;

private:
    Ui::PlaylistPicker *ui;
    QDialog *createPlaylistDialog;
    QLineEdit *playlistNameEdit;
#ifdef MAFW
    MafwPlaylistManagerAdapter *mafwPlaylistManager;
#endif

private slots:
    void onCreatePlaylist();
    void onCreatePlaylistAccepted();
    void onItemActivated(QListWidgetItem *item);
    void orientationChanged(int h, int w);
};

#endif // PLAYLISTPICKER_H
