#ifndef MUSICWINDOW_H
#define MUSICWINDOW_H

#include <QMainWindow>
#include <nowplayingwindow.h>
#include <QDir>
#include <QStringList>
#include <QDirIterator>
#include <QMenu>
#include <QtGui>
#include <share.h>
#ifdef Q_WS_MAEMO_5
#include <QMaemo5ValueButton>
#endif

#include "mafwrendereradapter.h"
#include "ui_musicwindow.h"

class MafwRendererAdapter;
namespace Ui {
    class MusicWindow;
}

enum UserRoles { UserRoleName=Qt::UserRole, UserRoleSongName };

class ListItemDelegate : public QStyledItemDelegate
{
public:
        explicit ListItemDelegate(QObject *parent=0) : QStyledItemDelegate(parent) {}
        virtual ~ListItemDelegate() {}

        void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
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

private slots:
    void onContextMenuRequested(const QPoint&);
    void onShareClicked();
};

#endif // MUSICWINDOW_H
