#include "frequencypickselector.h"

#include "frequencypickdialog.h"

FrequencyPickSelector::FrequencyPickSelector(uint min, uint max, uint step, uint frequency) :
    min(min),
    max(max),
    step(step)
{
    onSelected(frequency);
}

QString FrequencyPickSelector::currentValueText() const
{
    return QLocale().toString(frequency / 1000.0, 'f', FrequencyPickDialog::khzWidth(step)) + " MHz";
}

uint FrequencyPickSelector::currentFrequency() const
{
    return frequency;
}

QWidget* FrequencyPickSelector::widget(QWidget *parent)
{
    FrequencyPickDialog *dialog = new FrequencyPickDialog(parent, min, max, step, frequency);

    connect(dialog, SIGNAL(selected(uint)), this, SLOT(onSelected(uint)));

    return dialog;
}

void FrequencyPickSelector::onSelected(uint frequency)
{
    this->frequency = qBound(min, frequency, max);

    emit selected(currentValueText());
}
