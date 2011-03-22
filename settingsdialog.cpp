#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    if (QSettings().value("main/onApplicationExit").toString() == "stop-playback")
        ui->onExitBox->setCurrentIndex(1);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::accept()
{
    int index = ui->onExitBox->currentIndex();
    if (index == 0)
        QSettings().setValue("main/onApplicationExit", "do-nothing");
    else if (index == 1)
        QSettings().setValue("main/onApplicationExit", "stop-playback");
    this->close();
}
