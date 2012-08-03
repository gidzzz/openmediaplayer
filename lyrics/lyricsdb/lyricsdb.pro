 TEMPLATE      = lib
 CONFIG       += plugin
 INCLUDEPATH  += ..
 HEADERS       = lyricsdbplugin.h ../abstractlyricsprovider.h
 SOURCES       = lyricsdbplugin.cpp
 TARGET        = $$qtLibraryTarget(lyricsdbplugin)

 target.path = /opt/openmediaplayer/lyrics/

 INSTALLS += target
