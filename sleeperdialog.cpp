#include "sleeperdialog.h"
#include "ui_sleeperdialog.h"

SleeperDialog::SleeperDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SleeperDialog)
{
    ui->setupUi(this);
    ui->gridLayout_2->setContentsMargins(0,0,0,0);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Start"));
    ui->buttonBox->button(QDialogButtonBox::Reset)->setText(tr("Stop"));

    this->setAttribute(Qt::WA_DeleteOnClose);

    QMaemo5ListPickSelector *selector;
    QStandardItemModel *model;

    selector = new QMaemo5ListPickSelector;
    model = new QStandardItemModel(0, 1, selector);
    model->appendRow(new QStandardItem(tr("Stop playback")));
    model->appendRow(new QStandardItem(tr("Pause playback")));
    model->appendRow(new QStandardItem(tr("Close application")));
    selector->setModel(model);
    selector->setCurrentIndex(QSettings().value("timer/action").toString() == "stop-playback" ? 0 :
                              QSettings().value("timer/action").toString() == "pause-playback" ? 1 :
                              QSettings().value("timer/action").toString() == "close-application" ? 2 : 0);
    ui->actionBox->setPickSelector(selector);

    selector = new QMaemo5ListPickSelector;
    model = new QStandardItemModel(0, 1, selector);
    model->appendRow(new QStandardItem(tr("None")));
    model->appendRow(new QStandardItem(tr("Linear")));
    model->appendRow(new QStandardItem(tr("Exponential")));
    selector->setModel(model);
    selector->setCurrentIndex(QSettings().value("timer/volumeReduction").toString() == "none" ? NoReduction :
                              QSettings().value("timer/volumeReduction").toString() == "linear" ? LinearReduction :
                              QSettings().value("timer/volumeReduction").toString() == "exponential" ? ExponentialReduction : NoReduction);
    ui->volumeBox->setPickSelector(selector);

    ui->minutesBox->setValue(QSettings().value("timer/minutes", 30).toInt());

    refreshTimer = new QTimer(this);
    refreshTimer->setInterval(1000);

    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTitle()));
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButtonClicked(QAbstractButton*)));

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    orientationChanged(rotator->width(), rotator->height());
}

SleeperDialog::~SleeperDialog()
{
    delete ui;
}

void SleeperDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

void SleeperDialog::refreshTitle()
{
    this->setWindowTitle(tr("Sleep timer") + " " + time_mmss( (timeoutStamp/1000 - QDateTime::currentMSecsSinceEpoch()/1000) ));
}

void SleeperDialog::setTimeoutStamp(qint64 timeoutStamp)
{
    this->timeoutStamp = timeoutStamp;

    if (timeoutStamp == -1) {
        refreshTimer->stop();
        this->setWindowTitle(tr("Sleep timer"));
    } else {
        refreshTimer->start();
        refreshTitle();
    }
}

void SleeperDialog::onButtonClicked(QAbstractButton *button)
{
    if (button == ui->buttonBox->button(QDialogButtonBox::Ok)) {
        switch (static_cast<QMaemo5ListPickSelector*>(ui->actionBox->pickSelector())->currentIndex()) {
            case 0: QSettings().setValue("timer/action", "stop-playback"); break;
            case 1: QSettings().setValue("timer/action", "pause-playback"); break;
            case 2: QSettings().setValue("timer/action", "close-application"); break;
        }

        int reduction = static_cast<QMaemo5ListPickSelector*>(ui->volumeBox->pickSelector())->currentIndex();
        switch (reduction) {
            case NoReduction: QSettings().setValue("timer/volumeReduction", "none"); break;
            case LinearReduction: QSettings().setValue("timer/volumeReduction", "linear"); break;
            case ExponentialReduction: QSettings().setValue("timer/volumeReduction", "exponential"); break;
        }

        QSettings().setValue("timer/minutes", ui->minutesBox->value());

        emit timerRequested(ui->minutesBox->value()*60000, reduction);
    }

    else if (button == ui->buttonBox->button(QDialogButtonBox::Reset)) {
        emit timerRequested(-1, NoReduction);
    }
}

void SleeperDialog::orientationChanged(int w, int h)
{
    ui->gridLayout->removeWidget(ui->buttonBox);
    if (w < h) { // Portrait
        ui->gridLayout->addWidget(ui->buttonBox, 1, 0, 1, 1);
        ui->buttonBox->setSizePolicy(QSizePolicy::MinimumExpanding, ui->buttonBox->sizePolicy().verticalPolicy());
    } else { // Landscape
        ui->buttonBox->setSizePolicy(QSizePolicy::Maximum, ui->buttonBox->sizePolicy().verticalPolicy());
        ui->gridLayout->addWidget(ui->buttonBox, 0, 1, 1, 1, Qt::AlignBottom);
    }
    this->adjustSize();
}
