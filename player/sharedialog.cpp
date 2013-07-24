#include "sharedialog.h"

ShareDialog::ShareDialog(QStringList files, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShareDialog)
{
    ui->setupUi(this);

    this->files = files;

    connect(ui->bluetoothButton, SIGNAL(clicked()), this, SLOT(onBluetoothClicked()));
    connect(ui->emailButton, SIGNAL(clicked()), this, SLOT(onEmailClicked()));
}

ShareDialog::~ShareDialog()
{
    delete ui;
}

void ShareDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

void ShareDialog::onBluetoothClicked()
{
    for (int i = 0; i < files.size(); i++)
        files[i] = QUrl::fromLocalFile(files.at(i)).toEncoded();

    QDBusInterface("com.nokia.icd_ui",
                   "/com/nokia/bt_ui",
                   "com.nokia.bt_ui",
                   QDBusConnection::systemBus())
    .call("show_send_file_dlg", files);

    this->close();
}

void ShareDialog::onEmailClicked()
{
    for (int i = 0; i < files.size(); i++)
        files[i] = QUrl::fromLocalFile(files.at(i)).toEncoded();

    QDBusInterface("com.nokia.modest",
                   "/com/nokia/modest",
                   "com.nokia.modest",
                   QDBusConnection::sessionBus())
    .call("ComposeMail",
           QString(), // to
           QString(), // cc
           QString(), // bcc
           QString(), // subject
           QString(), // body
           files.join(","));

    this->close();
}
