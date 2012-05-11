#-------------------------------------------------
#
# Project created by QtCreator 2010-12-07T20:00:31
#
#-------------------------------------------------

QT += core gui dbus declarative opengl network

TARGET = openmediaplayer
TEMPLATE = app
TRANSLATIONS = lang/openmediaplayer.cs.ts \
               lang/openmediaplayer.de.ts \
               lang/openmediaplayer.en.ts \
               lang/openmediaplayer.es.ts \
               lang/openmediaplayer.fi.ts \
               lang/openmediaplayer.fr.ts \
               lang/openmediaplayer.hu.ts \
               lang/openmediaplayer.it.ts \
               lang/openmediaplayer.nl.ts \
               lang/openmediaplayer.pl.ts \
               lang/openmediaplayer.ro.ts \
               lang/openmediaplayer.ru.ts \
               lang/openmediaplayer.sv.ts \
               lang/openmediaplayer.tr.ts \
               lang/openmediaplayer.zh.ts

DEFINES += MAFW
INCLUDEPATH += /usr/lib/madde/linux-x86_64/sysroots/meego-core-armv7l-madde-sysroot-1.1-fs/usr/include/libmafw/

SOURCES += \
    main.cpp \
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
    qmlview.cpp \
    delegates/thumbnailitemdelegate.cpp \
    singlegenreview.cpp \
    singleplaylistview.cpp \
    aboutwindow.cpp \
    home.cpp \
    freqdlg.cpp \
    delegates/maintdelegate.cpp \
    editlyrics.cpp \
    tagwindow.cpp \
    mediaart.cpp \
    playlistquerymanager.cpp \
    upnpcontrol.cpp \
    upnpview.cpp \
    delegates/mediawithicondelegate.cpp \
    rotator.cpp

HEADERS += \
    mainwindow.h \
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
    qmlview.h \
    delegates/thumbnailitemdelegate.h \
    singlegenreview.h \
    singleplaylistview.h \
    aboutwindow.h \
    home.h \
    texteditautoresizer.h \
    freqdlg.h \
    delegates/maintdelegate.h \
    editlyrics.h \
    tagwindow.h \
    mediaart.h \
    playlistquerymanager.h \
    upnpcontrol.h \
    upnpview.h \
    delegates/mediawithicondelegate.h \
    rotator.h

FORMS += \
    mainwindow.ui \
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
    qmlview.ui \
    singlegenreview.ui \
    singleplaylistview.ui \
    aboutwindow.ui \
    home.ui \
    freqdlg.ui \
    editlyrics.ui \
    tagwindow.ui \
    upnpview.ui \
    upnpcontrol.ui

#generate translations
isEmpty(QMAKE_LRELEASE) {
  win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
  else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}

for(TSFILE, TRANSLATIONS) {
    exists($$TSFILE) {
	system($$QMAKE_LRELEASE $$TSFILE)
    }
}

symbian {
    TARGET.UID3 = 0xedf29700
    # TARGET.CAPABILITY += 
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}

unix:!symbian {

    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    BINDIR = $$PREFIX/bin
    DATADIR =$$PREFIX/share

    DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"

    maemo5 {
        QT += maemo5
        CONFIG += link_pkgconfig
        PKGCONFIG += dbus-1
        DEFINES += MAFW
        SOURCES += maemo5deviceevents.cpp \
                   fmtxdialog.cpp
        HEADERS += maemo5deviceevents.h \
                   fmtxdialog.h
        include(external-includepaths.pro)
    }

    INSTALLS += target desktop icon64

    target.path = $$BINDIR

    desktop.path = $$DATADIR/applications/hildon
    desktop.files += ../extra/$${TARGET}.desktop

    icon64.path = $$DATADIR/icons/hicolor/64x64/apps
    icon64.files += ../extra/$${TARGET}.png

}

contains(DEFINES, MAFW) {

    CONFIG += link_pkgconfig
    PKGCONFIG += mafw mafw-shared glib-2.0 gq-gconf gnome-vfs-2.0

    SOURCES += \
        mafw/mafwrenderersignalhelper.cpp \
        mafw/mafwsourcesignalhelper.cpp \
        mafw/mafwsourceadapter.cpp \
        mafw/mafwrendereradapter.cpp \
        mafw/mafwplaylistadapter.cpp \
        mafw/mafwplaylistmanageradapter.cpp \
        mafw/mafwadapterfactory.cpp

    HEADERS += \
        mafw/mafwrenderersignalhelper.h \
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
    qml_entertainmentview/entertainmentview.qml \
    qml_entertainmentview/Slider.qml \
    qml_carview/carview.qml \
    qml_carview/Button.qml \
    qml_carview/MetadataText.qml \
    qml_carview/Playlist.qml \
    qml_carview/Slider.qml \
    qml_carview/SongView.qml \
    qtc_packaging/meego.spec \
    lang/openmediaplayer.cs.qm \
    lang/openmediaplayer.de.qm \
    lang/openmediaplayer.en.qm \
    lang/openmediaplayer.es.qm \
    lang/openmediaplayer.fi.qm \
    lang/openmediaplayer.fr.qm \
    lang/openmediaplayer.hu.qm \
    lang/openmediaplayer.it.qm \
    lang/openmediaplayer.nl.qm \
    lang/openmediaplayer.pl.qm \
    lang/openmediaplayer.ro.qm \
    lang/openmediaplayer.ru.qm \
    lang/openmediaplayer.sv.qm \
    lang/openmediaplayer.tr.qm \
    lang/openmediaplayer.zh.qm

qml_entertainmentview.files += qml_entertainmentview/entertainmentview.qml
qml_entertainmentview.files += qml_entertainmentview/Slider.qml
qml_entertainmentview.path = /opt/openmediaplayer/qml/entertainmentview/

qml_carview.files += qml_carview/carview.qml
qml_carview.files += qml_carview/Button.qml
qml_carview.files += qml_carview/MetadataText.qml
qml_carview.files += qml_carview/Playlist.qml
qml_carview.files += qml_carview/Slider.qml
qml_carview.files += qml_carview/SongView.qml
qml_carview.path = /opt/openmediaplayer/qml/carview/

lang.files += lang/openmediaplayer.cs.qm
lang.files += lang/openmediaplayer.de.qm
lang.files += lang/openmediaplayer.en.qm
lang.files += lang/openmediaplayer.es.qm
lang.files += lang/openmediaplayer.fi.qm
lang.files += lang/openmediaplayer.fr.qm
lang.files += lang/openmediaplayer.hu.qm
lang.files += lang/openmediaplayer.it.qm
lang.files += lang/openmediaplayer.nl.qm
lang.files += lang/openmediaplayer.pl.qm
lang.files += lang/openmediaplayer.ro.qm
lang.files += lang/openmediaplayer.ru.qm
lang.files += lang/openmediaplayer.sv.qm
lang.files += lang/openmediaplayer.tr.qm
lang.files += lang/openmediaplayer.zh.qm
lang.path = /opt/openmediaplayer/lang/

INSTALLS += qml_entertainmentview
INSTALLS += qml_carview
INSTALLS += lang
