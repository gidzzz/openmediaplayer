#include "fmtxdialog.h"
#include "freqpickselector.h"
#include <QtMaemo5>

FMTXDialog::FMTXDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FMTXDialog)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    freqButton = new QMaemo5ValueButton("Frequency", this);
    ((QMaemo5ValueButton*)freqButton)->setValueLayout(QMaemo5ValueButton::ValueBesideText);
    ((QMaemo5ValueButton*)freqButton)->setPickSelector(new FreqPickSelector(this));
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
//    int lowest_integer = 76; // TODO: get this from fmtxd
//    int integer_value = 100; // TODO: get this from fmtxd
//    int fraction_value = 0; // TODO: get this from fmtxd

//    QListWidgetItem *selectedInteger = integers->item(integer_value - lowest_integer);
//    QListWidgetItem *selectedFraction = fractions->item(fraction_value);

//    integers->setCurrentItem(selectedInteger);
//    fractions->setCurrentItem(selectedFraction);

    QDialog::showEvent(event);
}

void FMTXDialog::onSaveClicked()
{
    if(ui->fmtxCheckbox->isChecked())
        system("fmtx_client -p 1");
    else
        system("fmtx_client -p 0");
    this->close();
}
