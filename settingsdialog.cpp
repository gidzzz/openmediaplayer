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
    if (QSettings().contains("FMTX/overrideChecks")) {
        if (!QSettings().value("FMTX/overrideChecks").toBool())
            ui->fmtxCheckBox->setChecked(true);
    }
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::accept()
{
    if (ui->onExitBox->currentIndex() == 0)
        QSettings().setValue("main/onApplicationExit", "do-nothing");
    else if (ui->onExitBox->currentIndex() == 1)
        QSettings().setValue("main/onApplicationExit", "stop-playback");
    QSettings().setValue("FMTX/overrideChecks", ui->fmtxCheckBox->isChecked());
    this->close();
}
