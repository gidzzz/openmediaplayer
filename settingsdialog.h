#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QDesktopWidget>

#include "nowplayingwindow.h"
#include "rotator.h"

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

private:
    Ui::SettingsDialog *ui;

private slots:
    void accept();
    void orientationChanged(int h, int w);
};

#endif // SETTINGSDIALOG_H
