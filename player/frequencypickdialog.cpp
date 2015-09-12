#include "frequencypickdialog.h"

#include <QPushButton>

#include "rotator.h"

FrequencyPickDialog::FrequencyPickDialog(QWidget *parent, uint min, uint max, uint step, uint frequency) :
    QDialog(parent),
    ui(new Ui::FrequencyPickDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Done"));

    this->setAttribute(Qt::WA_DeleteOnClose);

    // Prepare parameters
    this->step = step = (step ? step : 1);
    frequency = qBound(min, frequency, max);
    min /= 1000;
    max /= 1000;

    // Populate the MHz list
    for (uint mhz = min; mhz <= max; mhz++) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setTextAlignment(Qt::AlignCenter);
        item->setText(QString::number(mhz));
        ui->mhzList->addItem(item);
    }

    // Determine the format of kHz list entries
    const int width = khzWidth(step);
    const int div = width == 0 ? 1000
                  : width == 1 ? 100
                  : width == 2 ? 10
                  :              1;

    // Populate the kHz list
    for (uint khz = 0; khz < 1000; khz += step) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setTextAlignment(Qt::AlignCenter);
        item->setText(QString("%1").arg(khz/div, width, 10, QChar('0')));
        ui->khzList->addItem(item);
    }

    // Select items
    ui->mhzList->setCurrentRow(frequency/1000 - min);
    ui->khzList->setCurrentRow(frequency % 1000 / step);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    Rotator::acquire()->addClient(this);
}

FrequencyPickDialog::~FrequencyPickDialog()
{
    delete ui;
}

int FrequencyPickDialog::khzWidth(uint step)
{
    if (step % 1000 == 0)
        return 0;
    if (step % 100 == 0)
        return 1;
    if (step % 10 == 0)
        return 2;
    //  step % 1 == 0
        return 3;
}

void FrequencyPickDialog::onOrientationChanged(int w, int h)
{
    ui->dialogLayout->removeWidget(ui->buttonBox);
    if (w < h) { // Portrait
        ui->dialogLayout->addWidget(ui->buttonBox, 1, 0);
        ui->buttonBox->setSizePolicy(QSizePolicy::MinimumExpanding, ui->buttonBox->sizePolicy().verticalPolicy());
    } else { // Landscape
        ui->buttonBox->setSizePolicy(QSizePolicy::Maximum, ui->buttonBox->sizePolicy().verticalPolicy());
        ui->dialogLayout->addWidget(ui->buttonBox, 0, 1, 1, 1, Qt::AlignBottom);
    }
}

void FrequencyPickDialog::accept()
{
    const QList<QListWidgetItem*> mhz = ui->mhzList->selectedItems();
    const QList<QListWidgetItem*> khz = ui->khzList->selectedItems();

    if (!mhz.isEmpty() && !khz.isEmpty()) {
        emit selected(mhz.first()->text().toInt() * 1000 + ui->khzList->row(khz.first()) * step);
        QDialog::accept();
    }
}
