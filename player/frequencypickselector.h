#ifndef FREQUENCYPICKSELECTOR_H
#define FREQUENCYPICKSELECTOR_H

#include <QMaemo5AbstractPickSelector>

class FrequencyPickSelector : public QMaemo5AbstractPickSelector
{
    Q_OBJECT

public:
    FrequencyPickSelector(uint frequencyMin, uint frequencyMax, uint frequencyStep, uint frequency);

    QString currentValueText() const;
    uint currentFrequency() const;

    QWidget* widget(QWidget *parent);

signals:
    void selected(const QString &valueText);

private:
    const uint min;
    const uint max;
    const uint step;
    uint frequency;

private slots:
    void onSelected(uint frequency);
};

#endif // FREQUENCYPICKSELECTOR_H
