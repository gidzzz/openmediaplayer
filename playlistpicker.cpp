#include "playlistpicker.h"

PlaylistPicker::PlaylistPicker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlaylistPicker)
{
    ui->setupUi(this);

    QPushButton *newButton = new QPushButton(tr("New playlist"));
    ui->playlistList->insertItem(0, new QListWidgetItem());
    ui->playlistList->setItemWidget(ui->playlistList->item(0), newButton);

    connect(newButton, SIGNAL(clicked()), this, SLOT(onCreatePlaylist()));
    connect(ui->playlistList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemActivated(QListWidgetItem*)));

#ifdef MAFW
    MafwPlaylistManagerAdapter *mafwPlaylistManager = MafwPlaylistManagerAdapter::get();

    GArray* playlists = mafwPlaylistManager->listPlaylists();

    for (uint i = 0; i < playlists->len; i++) {
        MafwPlaylistManagerItem *item = &g_array_index(playlists, MafwPlaylistManagerItem, i);
        QString playlistName = QString::fromUtf8(item->name);

        if (playlistName != "FmpAudioPlaylist"
        && playlistName != "FmpVideoPlaylist"
        && playlistName != "FmpRadioPlaylist")
            (new QListWidgetItem(ui->playlistList))->setText(playlistName);
    }

    mafwPlaylistManager->freeListOfPlaylists(playlists);
#endif

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    orientationChanged(rotator->width(), rotator->height());
}

PlaylistPicker::~PlaylistPicker()
{
    delete ui;
}

void PlaylistPicker::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

void PlaylistPicker::onCreatePlaylist()
{
    createPlaylistDialog = new QDialog(this);
    createPlaylistDialog->setWindowTitle(tr("New playlist"));
    createPlaylistDialog->setAttribute(Qt::WA_DeleteOnClose);

    playlistNameEdit = new QLineEdit(createPlaylistDialog);
    playlistNameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save, Qt::Horizontal, this);
    buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save"));
    buttonBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onCreatePlaylistAccepted()));

    QHBoxLayout *layout = new QHBoxLayout(createPlaylistDialog);
    layout->addWidget(playlistNameEdit);
    layout->addWidget(buttonBox);

    createPlaylistDialog->show();
}

void PlaylistPicker::onCreatePlaylistAccepted()
{
    QString playlistName = playlistNameEdit->text();

    if (playlistName.isEmpty()) return;

#ifdef MAFW
    bool playlistExists = false;
    for (int i = 0; i < ui->playlistList->count(); i++) {
        if (ui->playlistList->item(i)->text() == playlistName) {
            playlistExists = true;
            break;
        }
    }

    MafwPlaylistManagerAdapter *mafwPlaylistManager = MafwPlaylistManagerAdapter::get();

    if (playlistExists) {
        if (ConfirmDialog(ConfirmDialog::OverwritePlaylist, this).exec() == QMessageBox::Yes) {
            createPlaylistDialog->close();
            mafwPlaylistManager->deletePlaylist(playlistName);
            mafwPlaylistManager->createPlaylist(playlistName);
            playlist = MAFW_PLAYLIST(mafwPlaylistManager->createPlaylist(playlistName));
            this->playlistName = playlistName;
            this->accept();
        }
    } else {
        createPlaylistDialog->close();
        mafwPlaylistManager->createPlaylist(playlistName);
        playlist = MAFW_PLAYLIST(mafwPlaylistManager->createPlaylist(playlistName));
        this->playlistName = playlistName;
        this->accept();
    }
#endif
}

void PlaylistPicker::orientationChanged(int w, int h)
{
    if (w < h) // Portrait
        this->setFixedHeight(680);
    else // Landscape
        this->setFixedHeight(360);
}

void PlaylistPicker::onItemActivated(QListWidgetItem *item)
{
    if (ui->playlistList->row(item) > 0) {
        playlist = MAFW_PLAYLIST(MafwPlaylistManagerAdapter::get()->createPlaylist(item->text()));
        this->playlistName = item->text();
        this->accept();
    }
}
