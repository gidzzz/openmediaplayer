#include "tagwindow.h"

TagWindow::TagWindow(QWidget *parent, QString objectId, QString title, QString artist, QString album) :
    QDialog(parent),
    ui(new Ui::TagWindow)
{
    ui->setupUi(this);
    this->objectId = objectId;
    ui->titleEdit->setText(title);
    ui->artistEdit->setText(artist);
    ui->albumEdit->setText(album);
}

TagWindow::~TagWindow()
{
    delete ui;
}

void TagWindow::on_saveButton_pressed()
{
    title = ui->titleEdit->text();
    artist = ui->artistEdit->text();
    album = ui->albumEdit->text();
    this->accept();
}
