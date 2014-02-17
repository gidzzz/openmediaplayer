#ifndef METADATAWATCHER_H
#define METADATAWATCHER_H

#include <QObject>
#include <QImage>
#include <QCryptographicHash>

#include "includes.h"

#include "mafw/mafwadapterfactory.h"

class MetadataWatcher: public QObject
{
    Q_OBJECT

public:
    MetadataWatcher(MafwAdapterFactory *factory);

    QMap<QString,QVariant> metadata();

signals:
    void metadataReady();
    void metadataChanged(QString key, QVariant value);

private:
    MafwRendererAdapter *mafwRenderer;
    MafwSourceAdapter *mafwSource;
    MafwSourceAdapter *mafwTrackerSource;

    QMap<QString,QVariant> currentMetadata;
    QMap<QString,QVariant> backupMetadata;
    QString currentObjectId;

    bool sourceMetadataPresent;

    void setMetadataFromSource(QString key, QVariant value);
    void setMetadataFromRenderer(QString key, QVariant value);

    static QVariant toQVariant(GValue *v);

private slots:
    void onStatusReceived(MafwPlaylist *, uint index, MafwPlayState, const char *objectId, QString);

    void onMediaChanged(int, char *objectId);
    void onSourceMetadataReceived(QString objectId, GHashTable *metadata, QString);
    void onSourceMetadataChanged(QString objectId);
    void onRendererMetadataReceived(GHashTable *metadata, QString objectId, QString);
    void onRendererMetadataChanged(QString metadata, QVariant value);
};

#endif // METADATAWATCHER_H