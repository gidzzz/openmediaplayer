#include "editlyrics.h"
#include "ui_editlyrics.h"
#include "nowplayingwindow.h"
#include "texteditautoresizer.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

EditLyrics::EditLyrics(QWidget *parent, QString d1, QString d2, QString d3) :
    QMainWindow(parent),
    ui(new Ui::EditLyrics)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5AutoOrientation);
#endif
    setAttribute(Qt::WA_DeleteOnClose);

    new TextEditAutoResizer(ui->content);

    modified = 0;
    ui->label->setText(d2 + " - " + d3);
    file = "/home/user/.lyrics/"+d1.replace("/","-");
    //qDebug() << file;

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

}

EditLyrics::~EditLyrics()
{
    delete ui;
}

void EditLyrics::on_pushButton_pressed()
{
    if ( QFileInfo(file).exists() )
        QFile::remove(file);
    QFile f(file);
    f.open( QIODevice::Truncate | QIODevice::Text | QIODevice::ReadWrite);
    QTextStream out(&f);
    out << ui->content->toPlainText();
    f.close();
    modified = 1;

    NowPlayingWindow * npw = qobject_cast<NowPlayingWindow*>(this->parentWidget());
    npw->reloadLyricsFromFile();

    this->close();
}
