#include "editlyrics.h"

EditLyrics::EditLyrics(QString artist, QString title, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditLyrics)
{
    ui->setupUi(this);

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    setAttribute(Qt::WA_DeleteOnClose);

    this->artist = artist;
    this->title = title;

    this->setWindowTitle(artist + " - " + title);

    new TextEditAutoResizer(ui->lyricsField);
    ui->lyricsField->setPlainText(LyricsManager::loadLyrics(artist, title));
    ui->lyricsField->setFocus();

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this), SIGNAL(activated()), this, SLOT(save()));
    connect(ui->saveButton, SIGNAL(pressed()), this, SLOT(save()));
}

EditLyrics::~EditLyrics()
{
    delete ui;
}

void EditLyrics::save()
{
    QString lyrics = ui->lyricsField->toPlainText();

    if (lyrics.isEmpty()) {
        MissionControl::acquire()->lyricsManager()->deleteLyrics(artist, title);
    } else {
        MissionControl::acquire()->lyricsManager()->storeLyrics(artist, title, lyrics);
    }

    this->close();
}
