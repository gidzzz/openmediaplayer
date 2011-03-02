#-------------------------------------------------
#
# Project created by QtCreator 2010-12-07T20:00:31
#
#-------------------------------------------------

QT       += core gui dbus

TARGET = mediaplayer
TEMPLATE = app
#DEFINES += DEBUG
DEFINES += MAFW


SOURCES += main.cpp \
    mainwindow.cpp \
    musicwindow.cpp \
    nowplayingwindow.cpp \
    videoswindow.cpp \
    internetradiowindow.cpp \
    share.cpp \
    videonowplayingwindow.cpp \
    cqgraphicsview.cpp \
    nowplayingindicator.cpp \
    delegates/songlistitemdelegate.cpp \
    delegates/artistlistitemdelegate.cpp \
    delegates/internetradiodelegate.cpp \
    delegates/playlistdelegate.cpp \
    radionowplayingwindow.cpp \
    qrotatedlabel.cpp \
    singlealbumview.cpp \
    delegates/singlealbumviewdelegate.cpp \
    singleartistview.cpp \
    mafwplaylistadapter.cpp \
    mafwplaylistmanageradapter.cpp

HEADERS  += mainwindow.h \
    musicwindow.h \
    nowplayingwindow.h \
    videoswindow.h \
    internetradiowindow.h \
    share.h \
    videonowplayingwindow.h \
    mirror.h \
    cqgraphicsview.h \
    nowplayingindicator.h \
    delegates/songlistitemdelegate.h \
    delegates/artistlistitemdelegate.h \
    delegates/internetradiodelegate.h \
    includes.h \
    delegates/playlistdelegate.h \
    radionowplayingwindow.h \
    qrotatedlabel.h \
    singlealbumview.h \
    delegates/singlealbumviewdelegate.h \
    singleartistview.h \
    mafwplaylistadapter.h \
    mafwplaylistmanageradapter.h

FORMS    += mainwindow.ui \
    musicwindow.ui \
    nowplayingwindow.ui \
    videoswindow.ui \
    internetradiowindow.ui \
    share.ui \
    fmtxdialog.ui \
    videonowplayingwindow.ui \
    nowplayingindicator.ui \
    radionowplayingwindow.ui \
    singlealbumview.ui \
    singleartistview.ui

CONFIG += mobility
MOBILITY = sensors

symbian {
    TARGET.UID3 = 0xedf29700
    # TARGET.CAPABILITY += 
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}

unix:!symbian {
    maemo5 {
        CONFIG += link_pkgconfig
        PKGCONFIG += mafw mafw-shared glib-2.0 gq-gconf
        QT += maemo5
        SOURCES +=     mafwrenderersignalhelper.cpp \
            mafwsourcesignalhelper.cpp \
            mafwsourceadapter.cpp \
            mafwrendereradapter.cpp \
            maemo5deviceevents.cpp \
            fmtxdialog.cpp \
            freqpickselector.cpp \
            qmaemo5rotator.cpp
        HEADERS +=    mafwrenderersignalhelper.h \
            mafwrendereradapter.h \
            mafwsourcesignalhelper.h \
            mafwsourceadapter.h \
            maemo5deviceevents.h \
            fmtxdialog.h \
            freqpickselector.h \
            qmaemo5rotator.h
        include(external-includepaths.pro)
        target.path = /opt/usr/bin
    } else {
        target.path = /usr/local/bin
    }
    INSTALLS += target
}
