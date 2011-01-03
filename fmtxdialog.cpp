#include "fmtxdialog.h"

FMTXDialog::FMTXDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FMTXDialog),
    selector(new FreqPickSelector(this))
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    freqButton = new QMaemo5ValueButton("Frequency", this);

    ((QMaemo5ValueButton*)freqButton)->setValueLayout(QMaemo5ValueButton::ValueBesideText);
    ((QMaemo5ValueButton*)freqButton)->setPickSelector(selector);
#else
    freqButton = new QPushButton("Frequency", this);
#endif
    ui->fmtxLayout->addWidget(freqButton);
    connect(ui->saveButton, SIGNAL(accepted()), this, SLOT(onSaveClicked()));
}

FMTXDialog::~FMTXDialog()
{
    delete ui;
}

void FMTXDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
}

void FMTXDialog::onSaveClicked()
{
    int frequency = selector->selectedFreq() * 1000;
#ifdef DEBUG
    qDebug() << "Selected Frequency:" << QString::number(frequency);
#endif
    if(ui->fmtxCheckbox->isChecked())
        // TODO: use DBus instead(!)
        system(QString("fmtx_client -p 1 -f %2 > /dev/null &").arg(frequency).toUtf8().constData());
    else
        system("fmtx_client -p 0 > /dev/null &");
    this->close();
}
