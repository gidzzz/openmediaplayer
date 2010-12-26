#include "fmtxdialog.h"
#include <QtMaemo5>

FMTXDialog::FMTXDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FMTXDialog)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    freqButton = new QMaemo5ValueButton("Frequency", this);
    ((QMaemo5ValueButton*)freqButton)->setValueLayout(QMaemo5ValueButton::ValueBesideText);
    ((QMaemo5ValueButton*)freqButton)->setValueText("100.00 MHz");
#else
    freqButton = new QPushButton("Frequency", this);
#endif
    ui->fmtxLayout->addWidget(freqButton);
    connect(ui->saveButton, SIGNAL(accepted()), this, SLOT(onSaveClicked()));
    connect(freqButton, SIGNAL(clicked()), this, SLOT(showFreqDialog()));

    // Creating the frequency selection dialog
    freqDialog = new QDialog(this);
    freqDialog->setWindowTitle("Select frequency");
    freqDialog->setMinimumHeight(350);
    QHBoxLayout *hLayout = new QHBoxLayout(freqDialog);
    integers = new QListWidget(freqDialog);
    fractions = new QListWidget(freqDialog);
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Vertical, freqDialog);
    hLayout->addWidget(integers);
    hLayout->addWidget(fractions);
    hLayout->addWidget(box);
    freqDialog->setLayout(hLayout);
    connect(box, SIGNAL(accepted()), freqDialog, SLOT(accept()));

    // TODO: get these values from fmtxd
    for (int i = 76; i <= 107; i++)
    {
        integers->addItem(QString::number(i));
    }
    for (int i = 0; i <= 9; i++)
    {
        fractions->addItem(QString::number(i));
    }
}

FMTXDialog::~FMTXDialog()
{
    delete ui;
}

void FMTXDialog::showEvent(QShowEvent *event)
{
    int lowest_integer = 76; // TODO: get this from fmtxd
    int integer_value = 100; // TODO: get this from fmtxd
    int fraction_value = 0; // TODO: get this from fmtxd

    QListWidgetItem *selectedInteger = integers->item(integer_value - lowest_integer);
    QListWidgetItem *selectedFraction = fractions->item(fraction_value);

    integers->setCurrentItem(selectedInteger);
    fractions->setCurrentItem(selectedFraction);

    QDialog::showEvent(event);
}

void FMTXDialog::showFreqDialog()
{
    freqDialog->exec();
    double theValue = integers->currentItem()->text().toDouble();
    theValue += fractions->currentItem()->text().toDouble() / 10;

    // TODO: Add code here that sets theValue to fmtxd

#ifdef Q_WS_MAEMO_5
    ((QMaemo5ValueButton*)freqButton)->setValueText(QString::number(theValue) + " MHz");
#endif
}

void FMTXDialog::onSaveClicked()
{
    if(ui->fmtxCheckbox->isChecked())
        system("fmtx_client -p 1");
    else
        system("fmtx_client -p 0");
    this->close();
}
