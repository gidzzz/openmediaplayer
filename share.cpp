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
    //qDebug() << "BluetoothTransfer::sendFile()" << file1;
    //QString sendCmd = "/opt/filebox/bt \"" + file1 + "\"";

    QString params = "";
    for (int i = 0; i < files.count(); ++i)
    {
        if ( QFileInfo(files.at(i)).isFile() )
        {
            QString path = files.at(i);
            path.replace("#", "%2523");
            path.replace(",", "%2C");
            path.replace(" ", "%20");

            if ( i == 0 )
                params += "\"file://" + path + "\"";
            else
                params += ",\"file://" + path + "\"";
        }
    }

    if ( params != "" )
    {
        QString sendCmd = "dbus-send --system --print-reply --dest='com.nokia.icd_ui' /com/nokia/bt_ui com.nokia.bt_ui.show_send_file_dlg array:string:" + params + " > /dev/null";
        system(sendCmd.toLatin1());
    }
    this->close();

}

void Share::on_share_mail_released()
{
    /*QMessage msg;
    QStringList fileList;
    fileList.append(file1);
    msg.appendAttachments(fileList);
    QMessageService service;
    service.send(msg);*/
    //QString sendCmd = "/opt/filebox/email \"" + file1 + "\"";

    QString params = "";
    for (int i = 0; i < files.count(); ++i)
    {
        if ( QFileInfo(files.at(i)).isFile() )
        {
            QString path = files.at(i);
            path.replace("#", "%2523");
            path.replace(",", "%2C");
            path.replace(" ", "%20");

            if ( i == 0 )
                params += "\"file://" + path + "\"";
            else
                params += ",\"file://" + path + "\"";
        }
    }

    if ( params != "" )
    {
        QString sendCmd = "dbus-send --type=method_call --dest=com.nokia.modest /com/nokia/modest com.nokia.modest.ComposeMail string:\"\" string:\"\" string:\"\" string:\"\" string:\"\" string:" + params;
        system(sendCmd.toLatin1());
    }
    this->close();

}
