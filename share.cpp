#include "share.h"

Share::Share(QWidget *parent, QStringList selected) :
    QDialog(parent),
    ui(new Ui::Share)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Share"));
    files = selected;
    ui->share_bt->setText(tr("Send via bluetooth"));
    ui->share_mail->setText(tr("Send via e-mail"));

}

Share::~Share()
{
    delete ui;
}


void Share::on_share_bt_released()
{
//    qDebug() << "BluetoothTransfer::sendFile()" << file1;
    //QString sended = "/opt/filebox/bt \"" + file1 + "\"";

    QString params="";
    for (int i=0; i < files.count(); ++i)
    {
        if ( QFileInfo(files.at(i)).isFile() )
        {
            QString archivo = files.at(i);
            archivo.replace("#","%2523");
            archivo.replace(",","%2C");
            archivo.replace(" ","%20");
            if ( i == 0 ) params += "\"file://" + archivo + "\"";
            else params += ",\"file://" + archivo + "\"";
        }
    }

    if ( params != "" )
    {
        QString sended = "dbus-send --system --print-reply --dest='com.nokia.icd_ui' /com/nokia/bt_ui com.nokia.bt_ui.show_send_file_dlg array:string:" + params;
        QByteArray ba = sended.toLatin1();
        const char *str1 = ba.data();
        system(str1);
    }
    this->close();

}

void Share::on_share_mail_released()
{
    /*QMessage msg;
    QStringList lista;
    lista.append(file1);
    msg.appendAttachments(lista);
    QMessageService msg2;
    msg2.send(msg);*/
    //QString sended = "/opt/filebox/email \"" + file1 + "\"";

    QString params="";
    for (int i=0; i < files.count(); ++i)
    {
        if ( QFileInfo(files.at(i)).isFile() )
        {
            QString archivo = files.at(i);
            archivo.replace("#","%2523");
            archivo.replace(",","%2C");
            archivo.replace(" ","%20");
            if ( i == 0 ) params += "\"file://" + archivo + "\"";
            else params += ",\"file://" + archivo + "\"";
        }
    }

    if ( params != "" )
    {
        QString sended = "dbus-send --type=method_call --dest=com.nokia.modest /com/nokia/modest com.nokia.modest.ComposeMail string:\"\" string:\"\" string:\"\" string:\"\" string:\"\" string:" + params;
        QByteArray ba = sended.toLatin1();
        const char *str1 = ba.data();
        system(str1);
    }
    this->close();

}
