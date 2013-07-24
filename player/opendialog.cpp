#include "opendialog.h"

OpenDialog::OpenDialog(QWidget *parent, bool simplified) :
    QDialog(parent),
    ui(new Ui::OpenDialog)
{
    ui->setupUi(this);

    ui->folderCheckBox->setChecked(QSettings().value("main/openFolders", false).toBool());

    connect(ui->replaceButton, SIGNAL(clicked()), this, SLOT(onButtonClicked()));

    if (simplified) {
        ui->replaceButton->setText(tr("Open"));
        ui->appendButton->hide();
    } else {
        connect(ui->appendButton, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
    }
}

OpenDialog::~OpenDialog()
{
    delete ui;
}

void OpenDialog::onButtonClicked()
{
    if (!ui->appendButton->isHidden())
        QSettings().setValue("main/appendSongs", this->sender() == ui->appendButton);

    QSettings().setValue("main/openFolders", ui->folderCheckBox->isChecked());
    QSettings().setValue("main/showOpenDialog", !ui->rememberCheckBox->isChecked());

    this->accept();
}
