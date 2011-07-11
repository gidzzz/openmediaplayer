#include "includes.h"
#include "home.h"
#include "ui_home.h"
#include "qsettings.h"
#include "qmaemo5rotator.h"
#include <qdir.h>
#include <qfileinfo.h>
#include "hildon-thumbnail/hildon-albumart-factory.h"

QString hPath;

Home::Home(QWidget *parent, QString target, QString path, QString album) :
    QDialog(parent),
    ui(new Ui::Home)
{
    ui->setupUi(this);

    albumtitle = album;
    target1 = target;
    QFileInfo dr(path);
    this->setWindowTitle(target);

    ui->pushButton->setIcon(QIcon("/usr/share/icons/"+currtheme+"/48x48/hildon/filemanager_folder_up.png"));
    ui->button->setIcon(QIcon("/usr/share/icons/"+currtheme+"/48x48/hildon/general_folder.png"));
    ui->button->setText( dr.fileName() );
    if ( path == "/") ui->button->setText( "/" );
    ui->button->setValueText( path );
    if ( path == "/") ui->button->setValueText( "" );

    CargarBrowser( path );

    setAttribute(Qt::WA_Maemo5AutoOrientation, true);

}

Home::~Home()
{
    delete ui;
}

void Home::CargarBrowser(QString directorio)
{

    hPath = directorio;
    QDir dir ( directorio, "*" );
    dir.setFilter ( QDir::Dirs | QDir::Hidden );
    if ( !dir.isReadable() )
          return;
    ui->button->setText( QFileInfo(directorio).fileName() );
    if ( directorio == "/") ui->button->setText( "/" );
    ui->button->setValueText( directorio );
    if ( directorio == "/") ui->button->setValueText( "" );

    ui->listWidget->clear();

    QStringList entries = dir.entryList();
    entries.sort();
    QStringList::ConstIterator it = entries.begin();


    QStringList filters;
    filters << "*.jpg";
    //dir.setNameFilters(filters);
    dir.setFilter( QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot );
    dir.setSorting( QDir::Name | QDir::DirsFirst | QDir::IgnoreCase );

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
      QFileInfo fileInfo = list.at(i);

        if ( fileInfo.isDir() )
        {
            QListWidgetItem *item1 = new QListWidgetItem( ui->listWidget );
            item1->setText(fileInfo.fileName());
            item1->setIcon(QIcon("/usr/share/icons/"+currtheme+"/48x48/hildon/general_folder.png"));
            ui->listWidget->insertItem( i, item1 );
        }
        else if ( fileInfo.completeSuffix() == "jpg" )
        {
            QListWidgetItem *item1 = new QListWidgetItem( ui->listWidget );
            item1->setText(fileInfo.fileName());
            item1->setIcon(QIcon("/usr/share/icons/"+currtheme+"/48x48/hildon/general_image.png"));
            ui->listWidget->insertItem( i, item1 );
        }

    }

    if ( ui->listWidget->count() > 0 ) {
        ui->listWidget->scrollToItem(ui->listWidget->item(0));
    }

}

void Home::on_listWidget_itemClicked(QListWidgetItem* item)
{
    QString temp = hPath;
    if ( temp != "/" ) temp.append("/");
    temp.append( item->text() );
    if ( temp == "//" ) temp="/";

    if ( QFileInfo(temp).isFile() )
    {
        //qDebug() << QString::fromUtf8(hildon_albumart_get_path(NULL,albumtitle.toUtf8(),"album"));
        QString newfile = QString::fromUtf8(hildon_albumart_get_path(NULL,albumtitle.toUtf8(),"album"));
        if ( QFileInfo(newfile).exists() )
            QFile::remove(newfile);
        QFile::copy(temp,newfile);
        newalbumart = newfile;
        this->accept();
    }

    CargarBrowser( temp );
}

void Home::on_pushButton_clicked()
{
    if ( hPath == "/" ) return;
    QString nPath = hPath;
    int i = nPath.lastIndexOf( "/" );
    nPath.remove ( i, nPath.length() - i );
    if ( nPath == "" ) nPath = "/";
    CargarBrowser( nPath );
}
