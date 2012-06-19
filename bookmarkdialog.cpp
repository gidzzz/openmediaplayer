#include "bookmarkdialog.h"

BookmarkDialog::BookmarkDialog(QWidget *parent, MafwAdapterFactory *factory, QString name, QString address, QString objectId) :
    QDialog(parent),
    ui(new Ui::BookmarkDialog)
{
    ui->setupUi(this);

#ifdef MAFW
    mafwRadioSource = factory->getRadioSource();
    this->objectId = objectId;

    this->setWindowTitle(objectId.isEmpty() ? tr("Add radio bookmark") : tr("Edit radio bookmark"));
#endif

    ui->nameBox->setText(name);
    ui->addressBox->setText(address.isEmpty() ? "http://" : address);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    orientationChanged(rotator->width(), rotator->height());
}

BookmarkDialog::~BookmarkDialog()
{
    delete ui;
}

void BookmarkDialog::accept()
{
    if (ui->nameBox->text().isEmpty()) {
#ifdef Q_WS_MAEMO_5
        QMaemo5InformationBox::information(this->parentWidget(), tr("Unable to add empty bookmark"));
#endif
    } else {
        if (ui->addressBox->text().indexOf("://") > 0 && !ui->addressBox->text().endsWith("://")) {

#ifdef MAFW
            GHashTable* metadata = mafw_metadata_new();
            mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_TITLE, ui->nameBox->text().toUtf8().data());
            mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_URI, ui->addressBox->text().toUtf8().data());
            mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_MIME, "audio/unknown");

            if (objectId.isEmpty())
                mafwRadioSource->createObject("iradiosource::", metadata);
            else
                mafwRadioSource->setMetadata(objectId.toUtf8(), metadata);

            mafw_metadata_release(metadata);
#endif

#ifdef Q_WS_MAEMO_5
            QMaemo5InformationBox::information(this->parentWidget(), tr("Media bookmark saved"));
#endif
            QDialog::accept();
        } else {
#ifdef Q_WS_MAEMO_5
            QMaemo5InformationBox::information(this->parentWidget(), tr("Invalid URL"));
#endif
        }
    }
}

void BookmarkDialog::orientationChanged(int w, int h)
{
    ui->gridLayout->removeWidget(ui->buttonBox);
    if (w < h) { // Portrait
        ui->gridLayout->addWidget(ui->buttonBox, 1, 0, 1, 1);
        ui->buttonBox->setSizePolicy(QSizePolicy::MinimumExpanding, ui->buttonBox->sizePolicy().verticalPolicy());
    } else { // Landscape
        ui->buttonBox->setSizePolicy(QSizePolicy::Maximum, ui->buttonBox->sizePolicy().verticalPolicy());
        ui->gridLayout->addWidget(ui->buttonBox, 0, 1, 1, 1, Qt::AlignBottom);
    }
    this->adjustSize();
}
