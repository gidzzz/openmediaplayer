#include "freqdlg.h"
#include "ui_freqdlg.h"
#include "qmaemo5rotator.h"

FreqDlg::FreqDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FreqDlg)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Done"));

    res = "";

    this->setWindowTitle(tr("Select frequency"));
    frequency = new GConfItem("/system/fmtx/frequency");
    refreshFreqValues();

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    this->orientationChanged();

}

FreqDlg::~FreqDlg()
{
    delete ui;
}

void FreqDlg::orientationChanged()
{
    ui->gridLayout_2->removeWidget(ui->buttonBox);
    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height()) {
        this->setMinimumHeight(680);
        this->setMaximumHeight(680);
        ui->gridLayout_2->addWidget(ui->buttonBox, 2, 0, 1, 2); // portrait
        ui->buttonBox->setSizePolicy(QSizePolicy::Minimum, ui->buttonBox->sizePolicy().verticalPolicy());
    } else {
        this->setMinimumHeight(350);
        this->setMaximumHeight(350);
        ui->buttonBox->setSizePolicy(QSizePolicy::Minimum, ui->buttonBox->sizePolicy().verticalPolicy());
        ui->gridLayout_2->addWidget(ui->buttonBox, 0, 1, 1, 1, Qt::AlignBottom); // landscape
    }
}

double FreqDlg::selectedFreq() const
{
    double selected = ui->integers->currentItem()->text().toDouble();
    selected += ui->fractions->currentItem()->text().toDouble() / 10;
    return selected;
}

void FreqDlg::setSelectedFreq(double d)
{
    int selectedInteger = d;
    int selectedFraction = qCeil((d - selectedInteger) * 10);
    ui->integers->setCurrentRow(selectedInteger - _minFreq);
    for (int i = 0; i < ui->fractions->count(); i++) {
        if (ui->fractions->item(i)->text().toInt() == selectedFraction) {
            ui->fractions->clearSelection();
            ui->fractions->setCurrentRow(i);
            ui->fractions->scrollToItem(ui->fractions->item(i));
            break;
        }
    }
}

QString FreqDlg::currentValueText() const
{
    return ui->integers->currentItem()->text() + "." + ui->fractions->currentItem()->text() + " " + tr("MHz");
}

void FreqDlg::refreshFreqValues()
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
    ui->integers->clear();
    ui->fractions->clear();
    for (int i = _minFreq; i <= _maxFreq; i++)
    {
        QListWidgetItem *item = new QListWidgetItem(ui->integers);
        item->setText(QString::number(i));
        item->setTextAlignment(Qt::AlignCenter);
        ui->integers->addItem(item);
    }

    for (int i = regionStepValue-1; i <= 9; i += regionStepValue)
    {
        QListWidgetItem *item = new QListWidgetItem(ui->fractions);
        item->setText(QString::number(i));
        item->setTextAlignment(Qt::AlignCenter);
        ui->fractions->addItem(item);
    }

    setSelectedFreq(selectedFreq);
}

void FreqDlg::onFrequencyChanged()
{
    this->setSelectedFreq(frequency->value().toDouble());
    //this->updateText();
}

QVariant FreqDlg::getValue(QString property)
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

void FreqDlg::setValue(QString property, QVariant value)
{
    QDBusMessage message = QDBusMessage::createMethodCall (FMTX_SERVICE, FMTX_OBJ_PATH, DBUS_INTERFACE_PROPERTIES, DBUS_PROPERTIES_SET);
    QList<QVariant> list;
    list << DBUS_INTERFACE_PROPERTIES << property << QVariant::fromValue(QDBusVariant(value));
    message.setArguments(list);

    QDBusReply<QDBusVariant> response = QDBusConnection::systemBus().call(message);

    QString error;

    if (property == "rds_text")
        error.append("Unable to set RDS info text: ");
    else if (property == "frequency")
        error.append("Unable to set frequency:");
    else if (property == "rds_ps")
        error.append("Unable to set RDS station name:");
    else if (property == "state")
        error.append("Unable to set FmTx state:");

    if (!response.isValid () && response.error().type() != QDBusError::InvalidSignature)
           qWarning () << error << response.error().message();
}

void FreqDlg::on_buttonBox_accepted()
{
    res = currentValueText();
    this->accept();
}
