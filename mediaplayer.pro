#-------------------------------------------------
#
# Project created by QtCreator 2010-12-07T20:00:31
#
#-------------------------------------------------

QT       += core gui dbus declarative opengl network

TARGET = mediaplayer
TEMPLATE = app
TRANSLATIONS = mediaplayer.ts

DEFINES += MAFW
INCLUDEPATH += /usr/lib/madde/linux-x86_64/sysroots/meego-core-armv7l-madde-sysroot-1.1-fs/usr/include/libmafw/

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
    delegates/playlistdelegate.cpp \
    radionowplayingwindow.cpp \
    singlealbumview.cpp \
    delegates/singlealbumviewdelegate.cpp \
    singleartistview.cpp \
    settingsdialog.cpp \
    entertainmentview.cpp \
    delegates/thumbnailitemdelegate.cpp \
    singlegenreview.cpp \
    singleplaylistview.cpp \
    aboutwindow.cpp \
    home.cpp \
    freqdlg.cpp \
    delegates/maintdelegate.cpp \
    editlyrics.cpp \
    tagwindow.cpp

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
    includes.h \
    delegates/playlistdelegate.h \
    radionowplayingwindow.h \
    singlealbumview.h \
    delegates/singlealbumviewdelegate.h \
    singleartistview.h \
    settingsdialog.h \
    entertainmentview.h \
    delegates/thumbnailitemdelegate.h \
    singlegenreview.h \
    singleplaylistview.h \
    aboutwindow.h \
    home.h \
    texteditautoresizer.h \
    freqdlg.h \
    delegates/maintdelegate.h \
    editlyrics.h \
    tagwindow.h

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
    entertainmentview.ui \
    singlegenreview.ui \
    singleplaylistview.ui \
    aboutwindow.ui \
    home.ui \
    freqdlg.ui \
    editlyrics.ui \
    tagwindow.ui

symbian {
    TARGET.UID3 = 0xedf29700
    # TARGET.CAPABILITY += 
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}

unix:!symbian {
    maemo5 {
        QT += maemo5
        CONFIG += link_pkgconfig
        PKGCONFIG += dbus-1
        DEFINES += MAFW
        SOURCES +=     maemo5deviceevents.cpp \
            fmtxdialog.cpp
        HEADERS +=    maemo5deviceevents.h \
            fmtxdialog.h
        include(external-includepaths.pro)
    }
    target.path = /usr/local/bin
    INSTALLS += target
}

contains(DEFINES, MAFW) {

    CONFIG += link_pkgconfig
    PKGCONFIG += mafw mafw-shared glib-2.0 gq-gconf gnome-vfs-2.0

    SOURCES +=      mafw/mafwrenderersignalhelper.cpp \
            mafw/mafwsourcesignalhelper.cpp \
            mafw/mafwsourceadapter.cpp \
            mafw/mafwrendereradapter.cpp \
            mafw/mafwplaylistadapter.cpp \
            mafw/mafwplaylistmanageradapter.cpp \
            mafw/mafwadapterfactory.cpp

    HEADERS +=      mafw/mafwrenderersignalhelper.h \
            mafw/mafwrendereradapter.h \
            mafw/mafwsourcesignalhelper.h \
            mafw/mafwsourceadapter.h \
            mafw/mafwplaylistadapter.h \
            mafw/mafwplaylistmanageradapter.h \
            mafw/mafwadapterfactory.h
}

LIBS += -lhildonthumbnail
PKGCONFIG += glib-2.0 gtk+-2.0

OTHER_FILES += \
    entertainmentview.qml \
    Slider.qml \
    qtc_packaging/meego.spec

qmlfiles.files += entertainmentview.qml
qmlfiles.files += Slider.qml
qmlfiles.path = /opt/mediaplayer/qml/
INSTALLS += qmlfiles
