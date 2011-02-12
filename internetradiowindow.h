#ifndef INTERNETRADIOWINDOW_H
#define INTERNETRADIOWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QLabel>
#include <QLayout>
#include <QDesktopWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QSpacerItem>
#include <QMessageBox>
#include <QDialogButtonBox>
#include "nowplayingindicator.h"
#ifdef Q_WS_MAEMO_5
    #include <QMaemo5InformationBox>
#endif

#include "ui_internetradiowindow.h"
#include "radionowplayingwindow.h"
#include "delegates/internetradiodelegate.h"
#include "includes.h"

#ifdef Q_WS_MAEMO_5
    #include "fmtxdialog.h"
#endif

namespace Ui {
    class InternetRadioWindow;
}

class InternetRadioWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit InternetRadioWindow(QWidget *parent = 0, MafwSourceAdapter* msa = 0);
    ~InternetRadioWindow();

private:
    Ui::InternetRadioWindow *ui;
    void connectSignals();
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    QDialog *bookmarkDialog;
    QLabel *nameLabel;
    QLabel *addressLabel;
    QPushButton *saveButton;
    QLineEdit *addressBox;
    QLineEdit *nameBox;
    QDialogButtonBox *buttonBox;
    RadioNowPlayingWindow *window;
#ifdef MAFW
    MafwSourceAdapter *mafwTrackerSource;
    unsigned int browseAllStationsId;
#endif

private slots:
    void showFMTXDialog();
    void showAddBookmarkDialog();
    void onSaveClicked();
    void onStationSelected();
    void orientationChanged();
#ifdef MAFW
    void listStations();
    void browseAllStations(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
#endif
};

#endif // INTERNETRADIOWINDOW_H
