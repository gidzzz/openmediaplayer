#ifndef LYRICSPROVIDERSDIALOG_H
#define LYRICSPROVIDERSDIALOG_H

#include <QDialog>

#include "ui_lyricsprovidersdialog.h"
#include "lyricsmanager.h"
#include "includes.h"
#include "rotator.h"

namespace Ui {
    class LyricsProvidersDialog;
}

class LyricsProvidersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LyricsProvidersDialog(QString state, QWidget *parent = 0);
    ~LyricsProvidersDialog();

    QString state();

private:
    Ui::LyricsProvidersDialog *ui;

private slots:
    void addProvider(QString name, bool active);
    void checkProvider(bool checked);
    void moveProviderUp();
    void moveProviderDown();
    void onProviderChanged(QListWidgetItem *item);

    void orientationChanged(int h, int w);
};

#endif // LYRICSPROVIDERSDIALOG_H
