/**************************************************************************
    This file is part of Open MediaPlayer
    Copyright (C) 2010-2011 Timur Kristof

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "freqpickselector.h"

FreqPickSelector::FreqPickSelector(QObject *parent) :
    QMaemo5AbstractPickSelector(parent),
    freqDialog(0)
{
    integers = new QListWidget();
    fractions = new QListWidget();
    frequency = new GConfItem("/system/fmtx/frequency");

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
    if(minValue == 87)
       minValue++;
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
    double selectedFreq = frequency->value().toDouble() / 1000;

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

void FreqPickSelector::onFrequencyChanged()
{
    this->setSelectedFreq(frequency->value().toDouble());
    this->updateText();
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
    QGridLayout *mainLayout = new QGridLayout(freqDialog);
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Vertical, freqDialog);
    mainLayout->addWidget(integers, 0, 0, 1, 1);
    mainLayout->addWidget(fractions, 0, 1, 1, 1);
    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height()) {
        freqDialog->setMinimumHeight(680);
        mainLayout->addWidget(box, 2, 0, 1, mainLayout->columnCount(), Qt::AlignBottom);
        box->setSizePolicy(QSizePolicy::MinimumExpanding, box->sizePolicy().verticalPolicy());
    } else {
        mainLayout->addWidget(box, 0, 2, 2, 1, Qt::AlignBottom);
    }
    freqDialog->setLayout(mainLayout);
    connect(box, SIGNAL(accepted()), freqDialog, SLOT(accept()));
    connect(freqDialog, SIGNAL(accepted()), this, SLOT(updateText()));
    connect(frequency, SIGNAL(valueChanged()), this, SLOT(onFrequencyChanged()));

    refreshFreqValues();

    return freqDialog;
}
