#ifndef INCLUDES_H
#define INCLUDES_H
#include <Qt>
#include <QSettings>
#include <QFileInfo>

extern QSettings settings;
extern QString currtheme;

extern QString musicIcon, videosIcon, radioIcon, shuffleIcon,
    defaultAlbumArt, defaultAlbumArtMedium, defaultVideoImage,
    volumeButtonIcon, albumImage, radioImage, shareButtonIcon, deleteButtonIcon;

// Defines
/*
#define musicIcon "/usr/share/icons/"+currtheme+"/164x164/hildon/mediaplayer_main_button_music.png"
#define videosIcon "/usr/share/icons/"+currtheme+"/164x164/hildon/mediaplayer_main_button_video.png"
#define radioIcon "/usr/share/icons/"+currtheme+"/164x164/hildon/mediaplayer_main_button_radio.png"
#define shuffleIcon "/usr/share/icons/"+currtheme+"/164x164/hildon/mediaplayer_main_button_shuffle.png"
#define defaultAlbumArt "/usr/share/icons/"+currtheme+"/64x64/hildon/mediaplayer_default_album.png"
#define defaultAlbumArtMedium "/usr/share/icons/"+currtheme+"/124x124/hildon/mediaplayer_default_album.png"
#define defaultVideoImage "/usr/share/icons/"+currtheme+"/124x124/hildon/general_video.png"
#define volumeButtonIcon "/usr/share/icons/"+currtheme+"/64x64/hildon/mediaplayer_volume.png"
#define albumImage "/usr/share/icons/"+currtheme+"/295x295/hildon/mediaplayer_default_album.png"
#define radioImage "/usr/share/icons/"+currtheme+"/295x295/hildon/mediaplayer_default_stream.png"
#define shareButtonIcon "/usr/share/icons/"+currtheme+"/48x48/hildon/general_share.png"
#define deleteButtonIcon "/usr/share/icons/"+currtheme+"/48x48/hildon/general_delete.png"
*/

#define idleFrame "/usr/share/icons/hicolor/scalable/hildon/mediaplayer_nowplaying_indicator_pause.png"
#define shuffleIcon124 "/usr/share/icons/hicolor/124x124/hildon/mediaplayer_default_shuffle.png"
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
#define wmCloseIcon "/etc/hildon/theme/images/wmBackIconPressed.png"

// Enums
enum UserRoles { UserRoleName=Qt::UserRole,
                 UserRoleSongName,
                 UserRoleSongTitle,
                 UserRoleSongAlbum,
                 UserRoleSongArtist,
                 UserRoleSongDuration,
                 UserRoleSongCount,
                 UserRoleSongIndex,
                 UserRoleAlbumCount,
                 UserRoleArtistCount,
                 UserRoleAlbumArt,
                 UserRoleSongDurationS,
                 UserRoleObjectID,
                 UserRoleSongURI,
                 UserRoleValueText
               };

#endif // INCLUDES_H
