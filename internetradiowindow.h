#ifndef INTERNETRADIOWINDOW_H
#define INTERNETRADIOWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSpacerItem>
#ifdef Q_WS_MAEMO_5
#include <libosso.h>
#endif

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
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *labelLayout;
    QVBoxLayout *lineEditLayout;
    QVBoxLayout *verticalLayout;
    QVBoxLayout *verticalLayout_1;
    QLabel *addressLabel;
    QPushButton *saveButton;
    QLineEdit *addressBox;
    QLineEdit *nameBox;
    QSpacerItem *verticalSpacer;

private slots:
    void showFMTXDialog();
    void showAddBookmarkDialog();
};

#endif // INTERNETRADIOWINDOW_H
