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
#ifdef Q_WS_MAEMO_5
#include <libosso.h>
#include <QMaemo5InformationBox>
#endif
#include "ui_internetradiowindow.h"

namespace Ui {
    class InternetRadioWindow;
}

class InternetRadioWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit InternetRadioWindow(QWidget *parent = 0);
    ~InternetRadioWindow();

private:
    Ui::InternetRadioWindow *ui;
#ifdef Q_WS_MAEMO_5
    osso_context_t *osso_context1;
#endif
    void connectSignals();
    QDialog *bookmarkDialog;
    QLabel *nameLabel;
    QLabel *addressLabel;
    QPushButton *saveButton;
    QLineEdit *addressBox;
    QLineEdit *nameBox;
    QDialogButtonBox *buttonBox;

private slots:
    void showFMTXDialog();
    void showAddBookmarkDialog();
    void onSaveClicked();
};

#endif // INTERNETRADIOWINDOW_H
