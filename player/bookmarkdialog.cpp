#include "bookmarkdialog.h"

BookmarkDialog::BookmarkDialog(QWidget *parent, MafwRegistryAdapter *mafwRegistry, Media::Type type, QString address, QString name, QString objectId) :
    QDialog(parent),
    ui(new Ui::BookmarkDialog)
{
    ui->setupUi(this);

    mafwRadioSource = mafwRegistry->source(MafwRegistryAdapter::Radio);
    this->objectId = objectId;

    this->setWindowTitle(objectId.isEmpty() ? tr("Add radio bookmark") : tr("Edit radio bookmark"));

    ui->nameBox->setText(name);
    ui->addressBox->setText(address.isEmpty() ? "http://" : address);
    ui->videoBox->setChecked(type == Media::Video);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    Rotator::acquire()->addClient(this);
}

BookmarkDialog::~BookmarkDialog()
{
    delete ui;
}

void BookmarkDialog::accept()
{
    if (ui->nameBox->text().isEmpty()) {
        QMaemo5InformationBox::information(this->parentWidget(), tr("Unable to add empty bookmark"));
    } else {
        if (ui->addressBox->text().indexOf("://") > 0 && !ui->addressBox->text().endsWith("://")) {
            GHashTable* metadata = mafw_metadata_new();
            mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_TITLE, ui->nameBox->text().toUtf8().data());
            mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_URI, ui->addressBox->text().toUtf8().data());
            mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_MIME, ui->videoBox->isChecked() ? "video/unknown" : "audio/unknown");

            if (objectId.isEmpty())
                mafwRadioSource->createObject("iradiosource::", metadata);
            else
                mafwRadioSource->setMetadata(objectId, metadata);

            mafw_metadata_release(metadata);

            QMaemo5InformationBox::information(this->parentWidget(), tr("Media bookmark saved"));
            QDialog::accept();
        } else {
            QMaemo5InformationBox::information(this->parentWidget(), tr("Invalid URL"));
        }
    }
}

void BookmarkDialog::onOrientationChanged(int w, int h)
{
    ui->mainLayout->removeWidget(ui->videoBox);
    ui->mainLayout->removeWidget(ui->buttonBox);
    if (w < h) { // Portrait
        ui->mainLayout->addWidget(ui->videoBox, 2, 0, 1, 2);
        ui->mainLayout->addWidget(ui->buttonBox, 3, 0, 1, 2);
        ui->buttonBox->setSizePolicy(QSizePolicy::MinimumExpanding, ui->buttonBox->sizePolicy().verticalPolicy());
    } else { // Landscape
        ui->buttonBox->setSizePolicy(QSizePolicy::Maximum, ui->buttonBox->sizePolicy().verticalPolicy());
        ui->mainLayout->addWidget(ui->videoBox, 0, 2, 1, 1);
        ui->mainLayout->addWidget(ui->buttonBox, 1, 2, 1, 1, Qt::AlignBottom);
    }
    this->adjustSize();
}
