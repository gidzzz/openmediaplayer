#ifndef METADATADIALOG_H
#define METADATADIALOG_H

#include <QDialog>
#include <QKeyEvent>
#include <QClipboard>
#include <QDateTime>
#include <QMap>

#include "ui_metadatadialog.h"
#include "includes.h"
#include "kbmenu.h"

#include "mafw/mafwsourceadapter.h"
#include "mafw/mafwutils.h"

class MetadataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MetadataDialog(QWidget *parent, const QMap<QString,QVariant> &metadata);
    explicit MetadataDialog(QWidget *parent, MafwSourceAdapter *mafwSource, const QString &objectId);
    ~MetadataDialog();

private:
    Ui::MetadataDialog *ui;

    QString objectId;

    void keyPressEvent(QKeyEvent *e);

    void init();
    void display(const QMap<QString,QVariant> &metadata);
    QString sizeString(double size);

private slots:
    void onMetadataReceived(const QString &objectId, GHashTable *metadata);
    void onContextMenuRequested(const QPoint &pos);
    void copyCurrentItem();
};

#endif // METADATADIALOG_H
