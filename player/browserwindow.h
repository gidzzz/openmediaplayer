#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include "basewindow.h"

#include <QTimer>

#include "ui_browserwindow.h"
#include "includes.h"
#include "rotator.h"
#include "headerawareproxymodel.h"

#include "mafw/mafwregistryadapter.h"

namespace Ui {
    class BrowserWindow;
}

class BrowserWindow : public BaseWindow
{
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent, MafwRegistryAdapter *mafwRegistry);
    ~BrowserWindow();

    bool eventFilter(QObject *, QEvent *e);

protected:
    Ui::BrowserWindow *ui;

    QStandardItemModel *objectModel;
    QSortFilterProxyModel *objectProxyModel;

    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

protected slots:
    void orientationInit();
    void onOrientationChanged(int w, int h);
    void onSearchHideButtonClicked();
    void onSearchTextChanged(QString);
    void onChildClosed();
};

#endif // BROWSERWINDOW_H
