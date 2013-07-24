#ifndef INCLUDES_H
#define INCLUDES_H

#include <Qt>
#include <QSettings>
#include <QFileInfo>

extern QString defaultAlbumImage;
extern QString defaultRadioImage;
extern QString volumeButtonIcon;

// Defines
#define musicIcon "mediaplayer_main_button_music"
#define videosIcon "mediaplayer_main_button_video"
#define radioIcon "mediaplayer_main_button_radio"
#define shuffleIcon "mediaplayer_main_button_shuffle"
#define defaultAlbumIcon "mediaplayer_default_album"
#define defaultShuffleIcon "mediaplayer_default_shuffle"
#define defaultVideoIcon "general_video"
#define bookmarkButtonIcon "general_add"
#define shareButtonIcon "general_share"
#define deleteButtonIcon "general_delete"
#define volumeButtonOverlayIcon "general_speaker"

#define backgroundImage "/etc/hildon/theme/mediaplayer/background.png"
#define prevButtonIcon "/etc/hildon/theme/mediaplayer/Back.png"
#define prevButtonPressedIcon "/etc/hildon/theme/mediaplayer/BackPressed.png"
#define playButtonIcon "/etc/hildon/theme/mediaplayer/Play.png"
#define pauseButtonIcon "/etc/hildon/theme/mediaplayer/Pause.png"
#define stopButtonIcon "/etc/hildon/theme/mediaplayer/Stop.png"
#define stopButtonPressedIcon "/etc/hildon/theme/mediaplayer/StopPressed.png"
#define nextButtonIcon "/etc/hildon/theme/mediaplayer/Forward.png"
#define nextButtonPressedIcon "/etc/hildon/theme/mediaplayer/ForwardPressed.png"
#define repeatButtonIcon "/etc/hildon/theme/mediaplayer/Repeat.png"
#define repeatButtonPressedIcon "/etc/hildon/theme/mediaplayer/RepeatPressed.png"
#define shuffleButtonIcon "/etc/hildon/theme/mediaplayer/Shuffle.png"
#define shuffleButtonPressed "/etc/hildon/theme/mediaplayer/ShufflePressed.png"
#define wmCloseIcon "/etc/hildon/theme/images/wmBackIcon.png"
#define wmEditIcon "/etc/hildon/theme/images/wmEditIcon.png"

// Enums
enum UserRoles { UserRoleName=Qt::UserRole,
                 UserRoleHeader,
                 UserRoleFilterString,
                 UserRoleTitle,
                 UserRoleSongTitle,
                 UserRoleSongAlbum,
                 UserRoleSongArtist,
                 UserRoleSongDuration,
                 UserRoleSongCount,
                 UserRoleSongIndex,
                 UserRoleAlbumCount,
                 UserRoleArtistCount,
                 UserRoleObjectID,
                 UserRoleMIME,
                 UserRoleValueText
               };

namespace Duration {
    const int Unknown = -1;
    const int Blank = -2;
}

namespace Media {
    enum Type {
        Audio,
        Video
    };
}

// Handy function to generate "mm:ss" time string, support for negative values
inline QString mmss_pos(int seconds)
{
    if (seconds < 0) {
        seconds = -seconds;
        return QString("-%1:%2").arg(seconds/60, 2, 10, QChar('0')).arg(seconds%60, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2").arg(seconds/60, 2, 10, QChar('0')).arg(seconds%60, 2, 10, QChar('0'));
    }
}

// Handy function to generate "mm:ss" time string, support for duration codes
inline QString mmss_len(int seconds)
{
    return seconds == Duration::Blank ? QString() :
           seconds == Duration::Unknown ? "--:--" :
           QString("%1:%2").arg(seconds/60, 2, 10, QChar('0')).arg(seconds%60, 2, 10, QChar('0'));
}

#endif // INCLUDES_H
