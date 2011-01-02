#include "musicwindow.h"

void ListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
        // Thanks to hqh for fapman, this code is based on the list in it.
        QString songName = index.data(UserRoleSongName).toString();
        QString songLength = "--:--";
        QString songArtistAlbum = "(unknown artist) / (unknown album)";

        painter->save();
        QRect r = option.rect;
        QFont f = painter->font();
        QPen defaultPen = painter->pen();
        QColor gray;
        gray = QColor(156, 154, 156);

        if( QApplication::desktop()->width() > QApplication::desktop()->height() )
        {
            // Landscape
            r = option.rect;
            f.setPointSize(18);
            painter->setFont(f);
            painter->drawText(30, r.top()+5, r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft, songName, &r);

            r = option.rect;
            f.setPointSize(13);
            painter->setFont(f);
            r.setBottom(r.bottom()-10);
            painter->setPen(QPen(gray));
            painter->drawText(30, r.top(), r.width(), r.height(), Qt::AlignBottom|Qt::AlignLeft, songArtistAlbum, &r);
            painter->setPen(defaultPen);;

            r = option.rect;
            r.setRight(r.right()-12);
            f.setPointSize(18);
            painter->setFont(f);
            painter->drawText(r, Qt::AlignVCenter|Qt::AlignRight, songLength, &r);
        } else {
            // Portrait
            r = option.rect;
            f.setPointSize(18);
            painter->setFont(f);
            painter->drawText(r.left()+5, r.top()+5, r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft, songName, &r);

            r = option.rect;
            f.setPointSize(13);
            painter->setFont(f);
            r.setBottom(r.bottom()-10);
            painter->setPen(QPen(gray));
            painter->drawText(r.left()+5, r.top(), r.width(), r.height(), Qt::AlignBottom|Qt::AlignLeft, songArtistAlbum, &r);
            painter->setPen(defaultPen);;

            r = option.rect;
            r.setRight(r.right()-12);
            f.setPointSize(18);
            painter->setFont(f);
            painter->drawText(r, Qt::AlignVCenter|Qt::AlignRight, songLength, &r);

        }
        painter->restore();
}

QSize ListItemDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
        return QSize(400, 70);
}

MusicWindow::MusicWindow(QWidget *parent, MafwRendererAdapter* mra) :
        QMainWindow(parent),
        ui(new Ui::MusicWindow),
        mafwrenderer(mra)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    shuffleAllButton = new QMaemo5ValueButton(this);
#else
    shuffleAllButton = new QPushButton(this);
#endif
    shuffleAllButton->setText(tr("Shuffle songs"));
    ui->songsLayout->removeWidget(ui->songList);
    ui->songsLayout->addWidget(shuffleAllButton);
    ui->songsLayout->addWidget(ui->songList);
    QMainWindow::setCentralWidget(ui->verticalLayoutWidget);
    QMainWindow::setWindowTitle(tr("Songs"));
    myNowPlayingWindow = new NowPlayingWindow(this, mafwrenderer);
    ListItemDelegate *delegate = new ListItemDelegate(ui->songList);
    ui->songList->setItemDelegate(delegate);
    this->listSongs();
    ui->songList->setContextMenuPolicy(Qt::CustomContextMenu);
    this->connectSignals();
#ifdef Q_WS_MAEMO_5
    int numberOfSongs = ui->songList->count();
    if(numberOfSongs == 1) {
        QString label = tr("song");
        shuffleAllButton->setValueText(QString::number(numberOfSongs) + " " + label);
    } else {
        QString label = tr("songs");
        shuffleAllButton->setValueText(QString::number(numberOfSongs) + " " + label);
    }
    shuffleAllButton->setValueLayout(QMaemo5ValueButton::ValueUnderTextCentered);
    shuffleAllButton->setIconSize(QSize(64, 64));
    shuffleAllButton->setIcon(QIcon(shuffleButtonIcon));
#endif
    shuffleAllButton->hide();
}

MusicWindow::~MusicWindow()
{
    delete ui;
}

void MusicWindow::selectSong()
{
    myNowPlayingWindow->onMetadataChanged(ui->songList->currentRow()+1,
                                          ui->songList->count(),
                                          //QString("Song name"),
                                          QString(ui->songList->currentItem()->text()),
                                          tr("(unknown album)"),
                                          tr("(unknown arist)")
                                          );
    myNowPlayingWindow->show();
}

void MusicWindow::listSongs()
{
    QDirIterator directory_walker(
#ifdef Q_WS_MAEMO_5
            "/home/user/MyDocs/.sounds",
#else
            "/home",
#endif
            QDir::Files | QDir::NoSymLinks,
            QDirIterator::Subdirectories);

    while(directory_walker.hasNext())
    {
        directory_walker.next();

        if(directory_walker.fileInfo().completeSuffix() == "mp3") {
            QListWidgetItem *songItem = new QListWidgetItem(directory_walker.fileName());
            songItem->setData(UserRoleSongName, directory_walker.fileName());
            //ui->songList->addItem(directory_walker.fileName());
            ui->songList->addItem(songItem);
            myNowPlayingWindow->listSongs(directory_walker.fileName());
        }
    }
}

void MusicWindow::connectSignals()
{
#ifdef Q_WS_MAEMO_5
    connect(ui->songList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectSong()));
#else
    connect(ui->songList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(selectSong()));
#endif
    connect(ui->songList, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onContextMenuRequested(const QPoint &)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
}

void MusicWindow::onContextMenuRequested(const QPoint &point)
{
    contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"));
    contextMenu->addAction(tr("Delete"));
    contextMenu->addAction(tr("Set as ringing tone"));
    contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    contextMenu->exec(point);
}

void MusicWindow::onShareClicked()
{
    // The code used here (share.(h/cpp/ui) was taken from filebox's source code
    // C) 2010. Matias Perez
    QStringList list;
    QString clip = "/home/user/MyDocs/.sounds/" + ui->songList->statusTip();
    list.append(clip);
    Share *share = new Share(this, list);
    share->setAttribute(Qt::WA_DeleteOnClose);
    share->show();
}

void MusicWindow::orientationChanged()
{
    ui->songList->scroll(1,1);
}
