/**************************************************************************
    This file is part of Open MediaPlayer
    Copyright (C) 2010 Matias Perez

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "sharedialog.h"

ShareDialog::ShareDialog(QWidget *parent, QStringList files) :
    QDialog(parent),
    ui(new Ui::ShareDialog)
{
    ui->setupUi(this);
    this->files = files;
}

ShareDialog::~ShareDialog()
{
    delete ui;
}

void ShareDialog::on_share_bt_released()
{
    QString params;

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

void ShareDialog::on_share_mail_released()
{
    QString params;

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
