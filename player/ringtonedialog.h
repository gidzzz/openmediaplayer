#ifndef RINGTONEDIALOG_H
#define RINGTONEDIALOG_H

#include "confirmdialog.h"

#include <QDBusInterface>
#include <QUrl>
#include <QMaemo5InformationBox>

#include "mafw/mafwsourceadapter.h"

class RingtoneDialog : public ConfirmDialog
{
    Q_OBJECT

public:
    explicit RingtoneDialog(QWidget *parent,
                            MafwSourceAdapter *mafwSource, QString objectId,
                            QString title, QString artist);

private:
    bool accepted;
    QString objectId;
    QString uri;

    void setRingtone();

private slots:
    void done(int r);
    void onUriReceived(QString objectId, QString uri);
};

#endif // RINGTONEDIALOG_H
