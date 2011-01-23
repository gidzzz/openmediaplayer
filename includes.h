#ifndef INCLUDES_H
#define INCLUDES_H
#include <Qt>

// Defines
#define musicIcon "/usr/share/icons/hicolor/164x164/hildon/mediaplayer_main_button_music.png"
#define videosIcon "/usr/share/icons/hicolor/164x164/hildon/mediaplayer_main_button_video.png"
#define radioIcon "/usr/share/icons/hicolor/164x164/hildon/mediaplayer_main_button_radio.png"
#define shuffleIcon "/usr/share/icons/hicolor/164x164/hildon/mediaplayer_main_button_shuffle.png"
#define backgroundImage "/etc/hildon/theme/mediaplayer/background.png"
#define shuffleButtonIcon "/etc/hildon/theme/mediaplayer/Shuffle.png"
#define defaultAlbumArt "/usr/share/icons/hicolor/64x64/hildon/mediaplayer_default_album.png"
#define idleFrame "/usr/share/icons/hicolor/scalable/hildon/mediaplayer_nowplaying_indicator_pause.png"
#define prevButtonIcon "/etc/hildon/theme/mediaplayer/Back.png"
#define prevButtonPressedIcon "/etc/hildon/theme/mediaplayer/BackPressed.png"
#define playButtonIcon "/etc/hildon/theme/mediaplayer/Play.png"
#define pauseButtonIcon "/etc/hildon/theme/mediaplayer/Pause.png"
#define nextButtonIcon "/etc/hildon/theme/mediaplayer/Forward.png"
#define nextButtonPressedIcon "/etc/hildon/theme/mediaplayer/ForwardPressed.png"
#define repeatButtonIcon "/etc/hildon/theme/mediaplayer/Repeat.png"
#define repeatButtonPressedIcon "/etc/hildon/theme/mediaplayer/RepeatPressed.png"
#define shuffleButtonIcon "/etc/hildon/theme/mediaplayer/Shuffle.png"
#define shuffleButtonPressed "/etc/hildon/theme/mediaplayer/ShufflePressed.png"
#define volumeButtonIcon "/usr/share/icons/hicolor/64x64/hildon/mediaplayer_volume.png"
#define albumImage "/usr/share/icons/hicolor/295x295/hildon/mediaplayer_default_album.png"
#define wmCloseIcon "/etc/hildon/theme/images/wmBackIconPressed.png"
#define shareButtonIcon "/usr/share/icons/hicolor/48x48/hildon/general_share.png"
#define deleteButtonIcon "/usr/share/icons/hicolor/48x48/hildon/general_delete.png"

// Enums
enum UserRoles { UserRoleName=Qt::UserRole,
                 UserRoleSongName,
                 UserRoleSongTitle,
                 UserRoleSongAlbum,
                 UserRoleSongArtist,
                 UserRoleSongDuration,
                 UserRoleSongCount,
                 UserRoleAlbumCount,
                 UserRoleSongDurationS,
                 UserRoleObjectID
                };
enum npSongUserRoles { npUserRoleName=Qt::UserRole, npUserRoleSongName };

#endif // INCLUDES_H
