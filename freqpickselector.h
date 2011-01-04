#ifndef FREQPICKSELECTOR_H
#define FREQPICKSELECTOR_H

#include <QtGui>
#include <QMaemo5AbstractPickSelector>
#include <QFile>
#include <GConfItem>

class FreqPickSelector : public QMaemo5AbstractPickSelector
{
    Q_OBJECT
    QDialog *freqDialog;
    QListWidget *integers;
    QListWidget *fractions;
    int _minFreq;
    int _maxFreq;
    GConfItem *frequency;

public:
    explicit FreqPickSelector(QObject *parent = 0);
    ~FreqPickSelector();

    QString currentValueText() const;
    QWidget *widget(QWidget *parent);
    double selectedFreq() const;
    void setSelectedFreq(double d);
    void refreshFreqValues();

private slots:
    void updateText();
    void onFrequencyChanged();
};

#endif // FREQPICKSELECTOR_H
