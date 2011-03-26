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
    for (int i = 0; i < fractions->count(); i++) {
        if (fractions->item(i)->text().toInt() == selectedFraction) {
            fractions->clearSelection();
            fractions->setCurrentRow(i);
            fractions->scrollToItem(fractions->item(i));
            break;
        }
    }
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
    int minValue = this->getValue("freq_min").toInt() / 1000;
    int maxValue = this->getValue("freq_max").toInt() / 1000;
    _minFreq = minValue;
    _maxFreq = maxValue;
    int regionStepValue = this->getValue("freq_step").toInt() / 100;
    double selectedFreq = this->getValue("frequency").toDouble() / 1000;
#ifdef DEBUG
    qDebug() << "Minimum FMTX value:" << QString::number(minValue);
    qDebug() << "Maximum FMTX value:" << QString::number(maxValue);
    qDebug() << "FMTX Region spacing:" << QString::number(regionStepValue);
    qDebug() << "FMTX Frequency:" << QString::number(selectedFreq);
#endif

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

QVariant FreqPickSelector::getValue(QString property)
{
    QDBusMessage message = QDBusMessage::createMethodCall (FMTX_SERVICE, FMTX_OBJ_PATH, DBUS_INTERFACE_PROPERTIES, DBUS_PROPERTIES_GET);
    QList<QVariant> list;
    list << DBUS_INTERFACE_PROPERTIES << property;
    message.setArguments(list);

    QDBusReply<QDBusVariant> response = QDBusConnection::systemBus().call(message);
    if (!response.isValid () && response.error().type() != QDBusError::InvalidSignature)
    {
           qWarning () << "Unable to get property" << property << ":" << response.error().message();
    }

    return response.value().variant();
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
    freqDialog->setWindowTitle(dgettext("osso-fm-transmitter", "fmtx_ti_select_frequency"));
    freqDialog->setMinimumHeight(370);
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
