 TEMPLATE      = lib
 CONFIG       += plugin
 INCLUDEPATH  += ..
 HEADERS       = sonataplugin.h ../abstractlyricsprovider.h
 SOURCES       = sonataplugin.cpp
 TARGET        = $$qtLibraryTarget(sonataplugin)

 target.path = /opt/openmediaplayer/lyrics/

 INSTALLS += target
