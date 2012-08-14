 TEMPLATE      = lib
 CONFIG       += plugin
 INCLUDEPATH  += ..
 HEADERS       = maelyricaplugin.h ../abstractlyricsprovider.h
 SOURCES       = maelyricaplugin.cpp
 TARGET        = $$qtLibraryTarget(maelyricaplugin)

 target.path = /opt/openmediaplayer/lyrics/

 INSTALLS += target
