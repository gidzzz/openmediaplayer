 TEMPLATE      = lib
 CONFIG       += plugin
 INCLUDEPATH  += ..
 HEADERS       = azlyricsplugin.h ../abstractlyricsprovider.h
 SOURCES       = azlyricsplugin.cpp
 TARGET        = $$qtLibraryTarget(azlyricsplugin)

 target.path = /opt/openmediaplayer/lyrics/

 INSTALLS += target
