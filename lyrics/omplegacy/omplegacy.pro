 TEMPLATE      = lib
 CONFIG       += plugin
 INCLUDEPATH  += ..
 HEADERS       = omplegacyplugin.h ../abstractlyricsprovider.h
 SOURCES       = omplegacyplugin.cpp
 TARGET        = $$qtLibraryTarget(omplegacyplugin)

 target.path = /opt/openmediaplayer/lyrics/

 INSTALLS += target
