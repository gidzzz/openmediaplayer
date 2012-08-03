#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QMaemo5ListPickSelector>
#include <QStandardItemModel>

#include "ui_settingsdialog.h"
#include "lyricsprovidersdialog.h"
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

    bool eventFilter(QObject*, QEvent *e);

private:
    Ui::SettingsDialog *ui;

    QString lyricsProviders;
    void setLyricsProviders(QString lyricsProviders);

private slots:
    void configureLyricsProviders();
    void accept();
    void orientationChanged(int h, int w);
};

#endif // SETTINGSDIALOG_H
