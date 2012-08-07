#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QMaemo5InformationBox>
#include <QMaemo5ListPickSelector>
#include <QStandardItemModel>
#include <QKeyEvent>

#include "ui_settingsdialog.h"
#include "confirmdialog.h"
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
    void keyPressEvent(QKeyEvent *e);

    QString lyricsProviders;
    void setLyricsProviders(QString lyricsProviders);

private slots:
    void configureLyricsProviders();
    void clearLyricsCache();
    void accept();
    void orientationChanged(int h, int w);
};

#endif // SETTINGSDIALOG_H
