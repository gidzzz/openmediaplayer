#ifndef INCLUDES_H
#define INCLUDES_H
#include <Qt>
#include <QSettings>
#include <QFileInfo>

extern QSettings settings;
extern QString currtheme;

extern QString albumImage, radioImage;

// Defines
#define musicIcon "mediaplayer_main_button_music"
#define videosIcon "mediaplayer_main_button_video"
#define radioIcon "mediaplayer_main_button_radio"
#define shuffleIcon "mediaplayer_main_button_shuffle"
#define defaultAlbumIcon "mediaplayer_default_album"
#define defaultShuffleIcon "mediaplayer_default_shuffle"
#define defaultVideoIcon "general_video"
#define shareButtonIcon "general_share"
#define deleteButtonIcon "general_delete"

//#define albumImage "/usr/share/icons/"+currtheme+"/295x295/hildon/mediaplayer_default_album.png"
//#define radioImage "/usr/share/icons/"+currtheme+"/295x295/hildon/mediaplayer_default_stream.png"

#define idleFrame "/usr/share/icons/hicolor/scalable/hildon/mediaplayer_nowplaying_indicator_pause.png"
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
#define volumeButtonIcon "/usr/share/icons/hicolor/64x64/hildon/mediaplayer_volume.png"
#define wmCloseIcon "/etc/hildon/theme/images/wmBackIconPressed.png"

// Enums
enum UserRoles { UserRoleName=Qt::UserRole,
                 UserRoleTitle,
                 UserRoleSongTitle,
                 UserRoleSongAlbum,
                 UserRoleSongArtist,
                 UserRoleSongDuration,
                 UserRoleSongCount,
                 UserRoleSongIndex,
                 UserRoleAlbumCount,
                 UserRoleArtistCount,
                 UserRoleAlbumArt,
                 UserRoleObjectID,
                 UserRoleMIME,
                 UserRoleValueText
               };

namespace Duration {
    enum e {
        Unknown = -1,
        Blank = -2
    };
}

// handy function to generate "mm:ss" time string
inline QString time_mmss(int seconds)
{
    if (seconds < 0) {
        seconds = -seconds;
        return QString("-%1:%2").arg(seconds/60, 2, 10, QChar('0')).arg(seconds%60, 2, 10, QChar('0'));
    } else
        return QString("%1:%2").arg(seconds/60, 2, 10, QChar('0')).arg(seconds%60, 2, 10, QChar('0'));
}

#endif // INCLUDES_H
