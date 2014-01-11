#ifndef SLEEPERDIALOG_H
#define SLEEPERDIALOG_H

#include <QTimer>
#include <QDialog>
#include <QSettings>
#include <QDateTime>
#include <QAbstractButton>
#include <QStandardItemModel>
#include <QMaemo5ListPickSelector>
#include <QKeyEvent>

#include "includes.h"
#include "rotator.h"
#include "missioncontrol.h"

namespace Ui {
    class SleeperDialog;
}

class SleeperDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SleeperDialog(QWidget *parent = 0);
    ~SleeperDialog();

public slots:
    void setTimeoutStamp(qint64 timeoutStamp = -1);

private:
    Ui::SleeperDialog *ui;

    QTimer *refreshTimer;
    qint64 timeoutStamp;

    void keyPressEvent(QKeyEvent *e);

private slots:
    void refreshTitle();
    void onButtonClicked(QAbstractButton *button);
    void orientationChanged(int h, int w);
};

#endif // SLEEPERDIALOG_H
