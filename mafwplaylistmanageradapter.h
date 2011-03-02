#ifndef MAFWPLAYLISTMANAGERADAPTER_H
#define MAFWPLAYLISTMANAGERADAPTER_H

#include <QObject>
#include <libmafw-shared/mafw-playlist-manager.h>

class MafwPlaylistManagerAdapter : public QObject
{
    Q_OBJECT
public:
    explicit MafwPlaylistManagerAdapter(QObject *parent = 0);

signals:

public slots:

private:
    MafwPlaylistManager* playlist_manager;

};

#endif // MAFWPLAYLISTMANAGERADAPTER_H
