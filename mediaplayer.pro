#-------------------------------------------------
#
# Project created by QtCreator 2010-12-07T20:00:31
#
#-------------------------------------------------

QT       += core gui dbus declarative opengl

TARGET = mediaplayer
TEMPLATE = app

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
    settingsdialog.cpp \
    entertainmentview.cpp \
    delegates/thumbnailitemdelegate.cpp

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
    settingsdialog.h \
    entertainmentview.h \
    delegates/thumbnailitemdelegate.h

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
    singleartistview.ui \
    settingsdialog.ui \
    entertainmentview.ui

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
        QT += maemo5
        DEFINES += MAFW
        SOURCES +=     maemo5deviceevents.cpp \
            fmtxdialog.cpp \
            freqpickselector.cpp \
            qmaemo5rotator.cpp
        HEADERS +=    maemo5deviceevents.h \
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

contains(DEFINES, MAFW) {

    CONFIG += link_pkgconfig
    PKGCONFIG += mafw mafw-shared glib-2.0 gq-gconf gtk-2-0

    SOURCES +=      mafwrenderersignalhelper.cpp \
            mafwsourcesignalhelper.cpp \
            mafwsourceadapter.cpp \
            mafwrendereradapter.cpp \
            mafwplaylistadapter.cpp \
            mafwplaylistmanageradapter.cpp \

    HEADERS +=      mafwrenderersignalhelper.h \
            mafwrendereradapter.h \
            mafwsourcesignalhelper.h \
            mafwsourceadapter.h \
            mafwplaylistadapter.h \
            mafwplaylistmanageradapter.h
}

OTHER_FILES += \
    entertainmentview.qml

RESOURCES +=
