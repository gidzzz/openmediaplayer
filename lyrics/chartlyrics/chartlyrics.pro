 TEMPLATE      = lib
 CONFIG       += plugin
 INCLUDEPATH  += ..
 HEADERS       = chartlyricsplugin.h ../abstractlyricsprovider.h
 SOURCES       = chartlyricsplugin.cpp
 TARGET        = $$qtLibraryTarget(chartlyricsplugin)

 target.path = /opt/openmediaplayer/lyrics/

 INSTALLS += target
