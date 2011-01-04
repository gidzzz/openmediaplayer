#ifndef MUSICWINDOW_H
#define MUSICWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QStringList>
#include <QDirIterator>
#include <QMenu>
#include <QtGui>

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5ValueButton>
#endif

#include "nowplayingwindow.h"
#include "share.h"
#include "ui_musicwindow.h"

#ifdef Q_WS_MAEMO_5
    #include "mafwrendereradapter.h"
#else
    class MafwRendererAdapter;
#endif

#define shuffleButtonIcon "/etc/hildon/theme/mediaplayer/Shuffle.png"
#define defaultAlbumArt "/usr/share/icons/hicolor/64x64/hildon/mediaplayer_default_album.png"

namespace Ui {
    class MusicWindow;
}

enum UserRoles { UserRoleName=Qt::UserRole, UserRoleSongName };

class SongListItemDelegate : public QStyledItemDelegate
{
public:
        explicit SongListItemDelegate(QObject *parent=0) : QStyledItemDelegate(parent) {}
        virtual ~SongListItemDelegate() {}

        void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

class ArtistListItemDelegate : public QStyledItemDelegate
{
public:
    explicit ArtistListItemDelegate(QObject *parent=0) : QStyledItemDelegate(parent) {}
    virtual ~ArtistListItemDelegate() {}

    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class MusicWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MusicWindow(QWidget *parent = 0, MafwRendererAdapter* mra = 0);
    ~MusicWindow();

public slots:
    void selectSong();

private:
    Ui::MusicWindow *ui;
    NowPlayingWindow *myNowPlayingWindow;
    QMenu *contextMenu;
    MafwRendererAdapter* mafwrenderer;
#ifdef Q_WS_MAEMO_5
    QMaemo5ValueButton *shuffleAllButton;
#else
    QPushButton *shuffleAllButton;
#endif
    void listSongs();
    void connectSignals();
    void populateMenuBar();
    void hideLayoutContents();
    void saveViewState(QVariant);
    void loadViewState();

private slots:
    void onContextMenuRequested(const QPoint&);
    void onShareClicked();
    void orientationChanged();
    void showAlbumView();
    void showPlayListView();
    void showArtistView();
    void showSongsView();
    void showGenresView();
};

#endif // MUSICWINDOW_H
