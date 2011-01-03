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
    QFile minFile("/sys/class/i2c-adapter/i2c-2/2-0063/region_bottom_frequency");
    minFile.open(QIODevice::ReadOnly);
    QTextStream minStream(&minFile);
    int minValue = minStream.readLine().toInt() / 1000;
    minFile.close();
    QFile maxFile("/sys/class/i2c-adapter/i2c-2/2-0063/region_top_frequency");
    maxFile.open(QIODevice::ReadOnly);
    QTextStream maxStream(&maxFile);
    int maxValue = maxStream.readLine().toInt() / 1000 - 1;
    maxFile.close();
    _minFreq = minValue;
    _maxFreq = maxValue;
    QFile regionStep("/sys/class/i2c-adapter/i2c-2/2-0063/region_channel_spacing");
    regionStep.open(QIODevice::ReadOnly);
    QTextStream regionStream(&regionStep);
    int regionStepValue = regionStream.readLine().toInt() / 100;
    regionStep.close();
#ifdef DEBUG
    qDebug() << "Minimum FMTX value: " << QString::number(minValue);
    qDebug() << "Maximum FMTX value: " << QString::number(maxValue);
    qDebug() << "FMTX Region spacing: " << QString::number(regionStepValue);
#endif
    // TODO: Get this from gconf: /system/fmtx/frequency
    double selectedFreq = 100;

    // Now updating the list widgets
    integers->clear();
    fractions->clear();
    for (int i = _minFreq; i <= _maxFreq; i++)
    {
        QListWidgetItem *item = new QListWidgetItem(integers);
        item->setText(QString::number(i));
        item->setTextAlignment(Qt::AlignCenter);
        integers->addItem(item);
    }

    for (int i = regionStepValue-1; i <= 9; i += regionStepValue)
    {
        QListWidgetItem *item = new QListWidgetItem(fractions);
        item->setText(QString::number(i));
        item->setTextAlignment(Qt::AlignCenter);
        fractions->addItem(item);
    }

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
    freqDialog->setMinimumHeight(360);
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
