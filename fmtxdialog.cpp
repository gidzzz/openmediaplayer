#include "fmtxdialog.h"

FMTXDialog::FMTXDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FMTXDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(ui->saveButton, SIGNAL(accepted()), this, SLOT(onSaveClicked()));
#ifdef Q_WS_MAEMO_5
    fmtxList = new QMaemo5ValueButton(tr("Frequency"), this);
    fmtxList->setValueText("100.00 MHz");
    fmtxList->setValueLayout(QMaemo5ValueButton::ValueBesideText);
    ui->fmtxLayout->addWidget(fmtxList);

    fmtxSelector = new QMaemo5ListPickSelector(this);
    fmtxItems = new QListView(this);
    //fmtxSelector->setModel(&fmtxItems);
    fmtxList->setPickSelector(fmtxSelector);
#endif
}

FMTXDialog::~FMTXDialog()
{
    delete ui;
}

void FMTXDialog::onSaveClicked()
{
    if(ui->fmtxCheckbox->isChecked())
        system("fmtx_client -p 1");
    else
        system("fmtx_client -p 0");
    this->close();
}
