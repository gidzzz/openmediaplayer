#include "editlyrics.h"

EditLyrics::EditLyrics(QWidget *parent, QString lyricsFile, QString artist, QString title) :
    QMainWindow(parent),
    ui(new Ui::EditLyrics)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    setAttribute(Qt::WA_DeleteOnClose);

    new TextEditAutoResizer(ui->content);

    ui->label->setText(artist + " - " + title);
    file = "/home/user/.lyrics/" + lyricsFile;

    if ( QFileInfo(file).exists() )
    {
        QString lines;
        QFile data(file);
        if (data.open(QFile::ReadOnly | QFile::Truncate))
        {
            QTextStream out(&data);
            while ( !out.atEnd() )
                lines += out.readLine()+"\n";
        }
        data.close();
        QApplication::processEvents();
        ui->content->setPlainText(lines);
    }

    ui->content->setFocus();
}

EditLyrics::~EditLyrics()
{
    delete ui;
}

void EditLyrics::on_pushButton_pressed()
{
    QFile f(file);
    f.open( QIODevice::Truncate | QIODevice::Text | QIODevice::ReadWrite);
    QTextStream out(&f);
    out << ui->content->toPlainText();
    f.close();

    qobject_cast<NowPlayingWindow*>(this->parentWidget())->reloadLyricsFromFile();

    this->close();
}
