#include "freqpickselector.h"

FreqPickSelector::FreqPickSelector(QObject *parent) :
    QMaemo5AbstractPickSelector(parent),
    freqDialog(0)
{
    integers = new QListWidget();
    fractions = new QListWidget();

    refreshFreqValues();
}

FreqPickSelector::~FreqPickSelector()
{
    integers->deleteLater();
    fractions->deleteLater();
}

double FreqPickSelector::selectedFreq() const
{
    double selected = integers->currentItem()->text().toDouble();
    selected += fractions->currentItem()->text().toDouble() / 10;
    return selected;
}

void FreqPickSelector::setSelectedFreq(double d)
{
    int selectedInteger = d;
    int selectedFraction = ((d - selectedInteger) * 10);
    integers->setCurrentRow(selectedInteger - _minFreq);
    fractions->setCurrentRow(selectedFraction);
}

void FreqPickSelector::updateText()
{
    emit selected(currentValueText());
}

QString FreqPickSelector::currentValueText() const
{
    return integers->currentItem()->text() + "." + fractions->currentItem()->text() + " " + tr("MHz");
}

void FreqPickSelector::refreshFreqValues()
{
    // TODO: get these values from fmtxd
    _minFreq = 76;
    _maxFreq = 107;
    double selectedFreq = 100;

    // Now updating the list widgets
    integers->clear();
    fractions->clear();
    for (int i = _minFreq; i <= _maxFreq; i++)
        integers->addItem(QString::number(i));
    for (int i = 0; i <= 9; i++)
        fractions->addItem(QString::number(i));

    setSelectedFreq(selectedFreq);
}

QWidget *FreqPickSelector::widget(QWidget *parent)
{
    if (freqDialog != 0)
    {
        integers->setParent(0);
        fractions->setParent(0);

        freqDialog->deleteLater();
    }
    // For some odd reason, making one QDialog and then re-setting the parent of that doesn't work.
    // So I'm just making a new dialog on every call now.
    freqDialog = new QDialog(parent);
    freqDialog->setWindowTitle("Select frequency");
    freqDialog->setMinimumHeight(350);
    QHBoxLayout *hLayout = new QHBoxLayout(freqDialog);
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Vertical, freqDialog);
    hLayout->addWidget(integers);
    hLayout->addWidget(fractions);
    hLayout->addWidget(box);
    freqDialog->setLayout(hLayout);
    connect(box, SIGNAL(accepted()), freqDialog, SLOT(accept()));
    connect(freqDialog, SIGNAL(accepted()), this, SLOT(updateText()));

    refreshFreqValues();

    return freqDialog;
}
