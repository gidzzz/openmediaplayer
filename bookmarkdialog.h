#ifndef BOOKMARKDIALOG_H
#define BOOKMARKDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#ifdef Q_WS_MAEMO_5
    #include <QMaemo5InformationBox>
#endif

#include "ui_bookmarkdialog.h"
#include "includes.h"
#include "rotator.h"

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#endif

namespace Ui {
    class BookmarkDialog;
}

class BookmarkDialog : public QDialog
{
    Q_OBJECT

public:
    enum MediaType {
        Audio,
        Video
    };

    explicit BookmarkDialog(QWidget *parent = 0, MafwAdapterFactory *factory = 0, MediaType type = Audio, QString address = "", QString name = "", QString objectId = "");
    ~BookmarkDialog();

private:
    Ui::BookmarkDialog *ui;
#ifdef MAFW
    MafwSourceAdapter *mafwRadioSource;
    QString objectId;
#endif

private slots:
    void accept();
    void orientationChanged(int h, int w);
};

#endif // BOOKMARKDIALOG_H
