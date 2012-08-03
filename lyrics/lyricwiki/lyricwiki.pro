 TEMPLATE      = lib
 CONFIG       += plugin
 INCLUDEPATH  += ..
 HEADERS       = lyricwikiplugin.h ../abstractlyricsprovider.h
 SOURCES       = lyricwikiplugin.cpp
 TARGET        = $$qtLibraryTarget(lyricwikiplugin)

 target.path = /opt/openmediaplayer/lyrics/

 INSTALLS += target
