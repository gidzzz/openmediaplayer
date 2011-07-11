#include "tagwindow.h"
#include "ui_tagwindow.h"
#include "mafw/mafwsourceadapter.h"
#include "mafw/mafwadapterfactory.h"
#include "nowplayingwindow.h"
#include "glib-2.0/glib/ghash.h"

TagWindow::TagWindow(QWidget *parent, QString d1, QString d2, QString d3, QString d4) :
    QDialog(parent),
    ui(new Ui::TagWindow)
{
    ui->setupUi(this);
    id = d1;
    artist = d2;
    album = d3;
    title = d4;
    ui->lineEdit->setText(artist);
    ui->lineEdit_2->setText(album);
    ui->lineEdit_3->setText(title);
}

TagWindow::~TagWindow()
{
    delete ui;
}


void TagWindow::on_pushButton_pressed()
{
    artist = ui->lineEdit->text().toUtf8();
    album = ui->lineEdit_2->text().toUtf8();
    title = ui->lineEdit_3->text().toUtf8();
    this->accept();
}
