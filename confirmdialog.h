#ifndef CONFIRMDIALOG_H
#define CONFIRMDIALOG_H

#include <QMessageBox>
#include <QAbstractButton>
#include <QKeyEvent>

class ConfirmDialog : public QMessageBox
{
    Q_OBJECT

public:
    enum Type {
        Delete,
        DeleteSong,
        DeleteVideo,
        DeletePlaylist,
        DeleteAll,
        ClearCurrent,
        ClearLyrics,
        OverwritePlaylist,
        Ringtone,
        ResetArt
    };

    ConfirmDialog(Type type, QWidget *parent = 0, QString title = QString(), QString artist = QString()) :
        QMessageBox(parent)
    {
        this->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        this->button(QMessageBox::Yes)->setText(tr("Yes"));
        this->button(QMessageBox::No)->setText(tr("No"));

        switch (type) {
            case Delete:
                this->setWindowTitle(" ");
                this->setText(tr("Delete selected item from device?"));
                break;

            case DeleteSong:
                this->setWindowTitle(tr("Delete song?"));
                this->setText(tr("Are you sure you want to delete this song?") + "\n\n" + title + "\n" + artist);
                break;

            case DeleteVideo:
                this->setWindowTitle(tr("Delete video?"));
                this->setText(tr("Are you sure you want to delete this video?"));
                break;

            case DeletePlaylist:
                this->setWindowTitle(" ");
                this->setText(tr("Delete playlist?"));
                break;

            case DeleteAll:
                this->setWindowTitle(" ");
                this->setText(tr("Delete all items shown in view?"));
                break;

            case ClearCurrent:
                this->setWindowTitle(" ");
                this->setText(tr("Clear all songs from now playing?"));
                break;

            case ClearLyrics:
                this->setWindowTitle(" ");
                this->setText(tr("Delete all downloaded lyrics?"));
                break;

            case OverwritePlaylist:
                this->setWindowTitle(" ");
                this->setText(tr("Playlist with the same name exists, overwrite?"));
                break;

            case Ringtone:
                this->setWindowTitle(" ");
                this->setText(tr("Are you sure you want to set this song as ringing tone?") + "\n\n" + title + "\n" + artist);
                break;

            case ResetArt:
                this->setWindowTitle(" ");
                this->setText(tr("Reset album art?"));
                break;
        }
    }

protected:
    void keyReleaseEvent(QKeyEvent *e)
    {
        switch (e->key()) {
            case Qt::Key_Y:
                this->button(QMessageBox::Yes)->animateClick(50);
                break;

            case Qt::Key_N:
                this->button(QMessageBox::No)->animateClick(50);
                break;

            case Qt::Key_Backspace:
                this->button(QMessageBox::Cancel)->click();
                break;
        }
    }
};

#endif // CONFIRMDIALOG_H
