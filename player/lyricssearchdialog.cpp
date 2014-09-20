#include "lyricssearchdialog.h"

LyricsSearchDialog::LyricsSearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LyricsSearchDialog)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_DeleteOnClose);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Search"));

    // As long as LyricsManager does not support background fetching, metadata
    // should be kept up-to-date.
    MetadataWatcher *mw = MissionControl::acquire()->metadataWatcher();
    connect(mw, SIGNAL(metadataChanged(QString,QVariant)), this, SLOT(onMetadataChanged(QString,QVariant)));
    QMap<QString,QVariant> metadata = mw->metadata();
    onMetadataChanged(MAFW_METADATA_KEY_ARTIST, metadata.value(MAFW_METADATA_KEY_ARTIST));
    onMetadataChanged(MAFW_METADATA_KEY_TITLE, metadata.value(MAFW_METADATA_KEY_TITLE));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(onOrientationChanged(int,int)));
    onOrientationChanged(rotator->width(), rotator->height());
}

LyricsSearchDialog::~LyricsSearchDialog()
{
    delete ui;
}

void LyricsSearchDialog::onOrientationChanged(int w, int h)
{
    ui->dialogLayout->removeWidget(ui->buttonBox);
    if (w < h) { // Portrait
        ui->dialogLayout->addWidget(ui->buttonBox, 1, 0);
        ui->buttonBox->setSizePolicy(QSizePolicy::MinimumExpanding, ui->buttonBox->sizePolicy().verticalPolicy());
    } else { // Landscape
        ui->buttonBox->setSizePolicy(QSizePolicy::Maximum, ui->buttonBox->sizePolicy().verticalPolicy());
        ui->dialogLayout->addWidget(ui->buttonBox, 0, 1);
    }
    this->adjustSize();
}

void LyricsSearchDialog::onMetadataChanged(QString key, QVariant value)
{
    if (key == MAFW_METADATA_KEY_ARTIST) {
        if (this->artist != value.toString())
            ui->artistEdit->setText(this->artist = value.toString());
    }
    else if (key == MAFW_METADATA_KEY_TITLE) {
        if (this->title != value.toString())
            ui->titleEdit->setText(this->title = value.toString());
    }
}

void LyricsSearchDialog::accept()
{
    MissionControl::acquire()->lyricsManager()->fetchLyrics(artist, title, ui->artistEdit->text(), ui->titleEdit->text());

    QDialog::accept();
}
