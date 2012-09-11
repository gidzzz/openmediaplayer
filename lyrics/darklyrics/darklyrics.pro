 TEMPLATE      = lib
 CONFIG       += plugin
 INCLUDEPATH  += ..
 HEADERS       = darklyricsplugin.h ../abstractlyricsprovider.h
 SOURCES       = darklyricsplugin.cpp
 TARGET        = $$qtLibraryTarget(darklyricsplugin)

 target.path = /opt/openmediaplayer/lyrics/

 INSTALLS += target
