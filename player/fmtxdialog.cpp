#include "fmtxdialog.h"

#include <QSettings>
#include <QMaemo5InformationBox>

#include "frequencypickselector.h"
#include "rotator.h"

FMTXDialog::FMTXDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FMTXDialog),
    fmtx(new FMTXInterface(this))
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save"));

    this->setAttribute(Qt::WA_DeleteOnClose);

    ui->frequencyButton->setPickSelector(new FrequencyPickSelector(fmtx->frequencyMin(),
                                                                   fmtx->frequencyMax(),
                                                                   fmtx->frequencyStep(),
                                                                   fmtx->frequency()));

    switch (fmtx->state()) {
        case FMTXInterface::Enabled:
            ui->stateBox->setChecked(true);
            break;
        case FMTXInterface::Unavailable:
            ui->stateBox->setEnabled(false);
            ui->stateBox->setText(tr("FM transmitter disabled"));
            break;
        default:
            // Disabled: simply not enabled
            // Error: previous attempt was failed, but another can be made (?)
            break;
    }

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(ui->stateBox, SIGNAL(clicked(bool)), this, SLOT(onStateToggled(bool)));

    Rotator::acquire()->addClient(this);
}

FMTXDialog::~FMTXDialog()
{
    delete ui;
}

void FMTXDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

void FMTXDialog::showError(const QString &message)
{
    QMaemo5InformationBox::information(this, message, QMaemo5InformationBox::NoTimeout);
}

void FMTXDialog::onOrientationChanged(int w, int h)
{
    ui->dialogLayout->removeWidget(ui->buttonBox);
    if (w < h) { // Portrait
        ui->dialogLayout->addWidget(ui->buttonBox, 1, 0);
        ui->buttonBox->setSizePolicy(QSizePolicy::MinimumExpanding, ui->buttonBox->sizePolicy().verticalPolicy());
    } else { // Landscape
        ui->buttonBox->setSizePolicy(QSizePolicy::Maximum, ui->buttonBox->sizePolicy().verticalPolicy());
        ui->dialogLayout->addWidget(ui->buttonBox, 0, 1, 1, 1, Qt::AlignBottom);
    }
    this->adjustSize();
}

void FMTXDialog::onStateToggled(bool enabled)
{
    if (enabled && !QSettings().value("FMTX/overrideChecks", false).toBool()) {
        switch (fmtx->startability()) {
            case FMTXInterface::Startable:
                return;
            case FMTXInterface::OfflineMode:
                // TODO: Disable offline mode after asking for permission
                break;
            case FMTXInterface::HeadphonesConnected:
                showError(tr("Unable to use FM transmitter while headset or TV out cable is connected.\n"
                             "Unplug cable to continue using FM transmitter."));
                break;
            case FMTXInterface::UsbConnected:
                showError(tr("Unable to use FM transmitter while USB is connected.\n"
                             "Unplug USB to continue using FM transmitter."));
                break;
            default:
                break;
        }
        ui->stateBox->setChecked(false);
    }
}

void FMTXDialog::accept()
{
    fmtx->setFrequency(static_cast<FrequencyPickSelector*>(ui->frequencyButton->pickSelector())->currentFrequency());
    fmtx->setEnabled(ui->stateBox->isChecked());

    QDialog::accept();
}
