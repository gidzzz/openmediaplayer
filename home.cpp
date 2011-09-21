#include "home.h"

QString currentPath;

Home::Home(QWidget *parent, QString target, QString path, QString album) :
    QDialog(parent),
    ui(new Ui::Home)
{
    ui->setupUi(this);

    this->album = album;
    this->target = target;
    QFileInfo dr(path);
    this->setWindowTitle(target);

    // currtheme is "default" when it should be "hicolor"?
    qDebug() << "/usr/share/icons/"+currtheme+"/48x48/hildon/filemanager_folder_up.png";
    ui->pushButton->setIcon( QIcon("/usr/share/icons/"+currtheme+"/48x48/hildon/filemanager_folder_up.png") );
    ui->button->setIcon( QIcon("/usr/share/icons/"+currtheme+"/48x48/hildon/general_folder.png") );
    ui->button->setText( dr.fileName() );
    if ( path == "/") ui->button->setText( "/" );
    ui->button->setValueText( path );
    if ( path == "/") ui->button->setValueText( "" );

    openBrowser( path );

    setAttribute(Qt::WA_Maemo5AutoOrientation, true);

}

Home::~Home()
{
    delete ui;
}

void Home::openBrowser(QString directory)
{

    currentPath = directory;
    QDir dir ( directory, "*" );
    dir.setFilter ( QDir::Dirs | QDir::Hidden );
    if ( !dir.isReadable() )
          return;
    ui->button->setText( QFileInfo(directory).fileName() );
    if ( directory == "/") ui->button->setText( "/" );
    ui->button->setValueText( directory );
    if ( directory == "/") ui->button->setValueText( "" );

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
            QListWidgetItem *item = new QListWidgetItem( ui->listWidget );
            item->setText(fileInfo.fileName());
            item->setIcon(QIcon( "/usr/share/icons/"+currtheme+"/48x48/hildon/general_folder.png") );
            ui->listWidget->insertItem( i, item );
        }
        else if ( fileInfo.completeSuffix() == "jpg" )
        {
            QListWidgetItem *item = new QListWidgetItem( ui->listWidget );
            item->setText(fileInfo.fileName());
            item->setIcon( QIcon("/usr/share/icons/"+currtheme+"/48x48/hildon/general_image.png") );
            ui->listWidget->insertItem( i, item );
        }

    }

    if ( ui->listWidget->count() > 0 ) {
        ui->listWidget->scrollToItem(ui->listWidget->item(0));
    }

}

void Home::on_listWidget_itemClicked(QListWidgetItem* item)
{
    QString newPath = currentPath;
    if ( newPath != "/" ) newPath.append("/");
    newPath.append( item->text() );
    if ( newPath == "//" ) newPath = "/";

    if ( QFileInfo(newPath).isFile() )
    {
        newAlbumArt = MediaArt::setAlbumImage(album, newPath);
        this->accept();
    }

    openBrowser( newPath );
}

void Home::on_pushButton_clicked()
{
    if ( currentPath == "/" ) return;

    QString newPath = currentPath;
    int i = newPath.lastIndexOf( "/" );
    newPath.remove ( i, newPath.length() - i );
    if ( newPath == "" ) newPath = "/";
    openBrowser( newPath );
}
