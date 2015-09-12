#include "coverpicker.h"

CoverPicker::CoverPicker(QString album, QString path, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CoverPicker)
{
    ui->setupUi(this);
    ui->locationButton->setIcon(QIcon::fromTheme("filemanager_folder_up"));

    this->album = album;

    connect(ui->fileList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemActivated(QListWidgetItem*)));
    connect(ui->locationButton, SIGNAL(clicked()), this, SLOT(browse()));

    QStringList filters;
    dir.setNameFilters(filters << "*.jpg");
    dir.setFilter(QDir::AllDirs | QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

    browse(path);
}

CoverPicker::~CoverPicker()
{
    delete ui;
}

void CoverPicker::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

void CoverPicker::browse(QString path)
{
    if (!dir.cd(path)) return;

    path = dir.absolutePath();
    if (path == "/") {
        ui->locationButton->setText("/");
        ui->locationButton->setValueText(QString());
    } else {
        ui->locationButton->setText(dir.dirName());
        ui->locationButton->setValueText(path.left(path.lastIndexOf("/")+1));
    }

    ui->fileList->clear();

    QFileInfoList entries = dir.entryInfoList();
    foreach (QFileInfo entry, entries) {
        QListWidgetItem *item = new QListWidgetItem(ui->fileList);
        item->setText(entry.fileName());
        item->setIcon(QIcon::fromTheme(entry.isDir() ? "general_folder" : "general_image"));
    }

    ui->fileList->scrollToTop();
}

void CoverPicker::onItemActivated(QListWidgetItem* item)
{
    QString path = dir.absoluteFilePath(item->text());

    if (QFileInfo(path).isFile()) {
        cover = path;
        this->accept();
    } else {
        browse(path);
    }
}
