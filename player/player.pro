QT += core gui dbus declarative opengl network maemo5

TARGET = openmediaplayer
TEMPLATE = app

INCLUDEPATH += ../lyrics

DEFINES += MAFW MAFW_WORKAROUNDS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    musicwindow.cpp \
    nowplayingwindow.cpp \
    videoswindow.cpp \
    internetradiowindow.cpp \
    sharedialog.cpp \
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
    coverpicker.cpp \
    freqdlg.cpp \
    delegates/maindelegate.cpp \
    editlyrics.cpp \
    mediaart.cpp \
    playlistquerymanager.cpp \
    upnpcontrol.cpp \
    upnpview.cpp \
    delegates/mediawithicondelegate.cpp \
    rotator.cpp \
    playlistpicker.cpp \
    sleeperdialog.cpp \
    bookmarkdialog.cpp \
    lyricsmanager.cpp \
    lyricsprovidersdialog.cpp \
    delegates/shufflebuttondelegate.cpp \
    delegates/providerlistitemdelegate.cpp \
    basewindow.cpp \
    opendialog.cpp \
    currentplaylistmanager.cpp \
    maemo5deviceevents.cpp \
    fmtxdialog.cpp

HEADERS += \
    mainwindow.h \
    musicwindow.h \
    nowplayingwindow.h \
    videoswindow.h \
    internetradiowindow.h \
    sharedialog.h \
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
    coverpicker.h \
    texteditautoresizer.h \
    freqdlg.h \
    delegates/maindelegate.h \
    editlyrics.h \
    mediaart.h \
    playlistquerymanager.h \
    upnpcontrol.h \
    upnpview.h \
    delegates/mediawithicondelegate.h \
    rotator.h \
    playlistpicker.h \
    sleeperdialog.h \
    bookmarkdialog.h \
    lyrics/abstractlyricsprovider.h \
    lyricsmanager.h \
    lyricsprovidersdialog.h \
    headerawareproxymodel.h \
    confirmdialog.h \
    delegates/shufflebuttondelegate.h \
    delegates/providerlistitemdelegate.h \
    fastlistview.h \
    basewindow.h \
    kbmenu.h \
    opendialog.h \
    currentplaylistmanager.h \
    maemo5deviceevents.h \
    fmtxdialog.h

FORMS += \
    mainwindow.ui \
    musicwindow.ui \
    nowplayingwindow.ui \
    videoswindow.ui \
    internetradiowindow.ui \
    sharedialog.ui \
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
    coverpicker.ui \
    freqdlg.ui \
    editlyrics.ui \
    upnpview.ui \
    playlistpicker.ui \
    sleeperdialog.ui \
    bookmarkdialog.ui \
    lyricsprovidersdialog.ui \
    opendialog.ui

OTHER_FILES += \
    qml_entertainmentview/entertainmentview.qml \
    qml_entertainmentview/Slider.qml \
    qml_carview/carview.qml \
    qml_carview/Button.qml \
    qml_carview/MetadataText.qml \
    qml_carview/Playlist.qml \
    qml_carview/Slider.qml \
    qml_carview/SongView.qml

contains(DEFINES, MAFW) {
    CONFIG += link_pkgconfig
    PKGCONFIG += mafw mafw-shared glib-2.0 gq-gconf gnome-vfs-2.0 libplayback-1

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

LIBS += -lhildonthumbnail -lX11
CONFIG += link_pkgconfig
PKGCONFIG += dbus-1 gtk+-2.0

TRANSLATIONS = \
    lang/openmediaplayer.ar_SA.ts \
    lang/openmediaplayer.bg.ts \
    lang/openmediaplayer.cs.ts \
    lang/openmediaplayer.de.ts \
    lang/openmediaplayer.en.ts \
    lang/openmediaplayer.es.ts \
    lang/openmediaplayer.fi.ts \
    lang/openmediaplayer.fr.ts \
    lang/openmediaplayer.hu.ts \
    lang/openmediaplayer.it.ts \
    lang/openmediaplayer.nl.ts \
    lang/openmediaplayer.pl.ts \
    lang/openmediaplayer.pt_BR.ts \
    lang/openmediaplayer.pt_PT.ts \
    lang/openmediaplayer.ro.ts \
    lang/openmediaplayer.ru.ts \
    lang/openmediaplayer.sk.ts \
    lang/openmediaplayer.sv.ts \
    lang/openmediaplayer.tr.ts \
    lang/openmediaplayer.uk.ts \
    lang/openmediaplayer.zh.ts

isEmpty(QMAKE_LRELEASE) {
    QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
lrelease.input = TRANSLATIONS
lrelease.output = ${QMAKE_FILE_BASE}.qm
lrelease.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_BASE}.qm
lrelease.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += lrelease

isEmpty(PREFIX) {
    PREFIX = /usr
}
BINDIR = $$PREFIX/bin
DATADIR = $$PREFIX/share
PKGDATADIR = /opt/$$TARGET

INSTALLS += target
target.path = $$BINDIR

INSTALLS += desktop
desktop.path = $$DATADIR/applications/hildon
desktop.files += ../extra/$${TARGET}.desktop

INSTALLS += icon64
icon64.path = $$DATADIR/icons/hicolor/64x64/apps
icon64.files += ../extra/$${TARGET}.png

INSTALLS += qml_entertainmentview
qml_entertainmentview.files += qml_entertainmentview/entertainmentview.qml
qml_entertainmentview.files += qml_entertainmentview/Slider.qml
qml_entertainmentview.path = $$PKGDATADIR/qml/entertainmentview/

INSTALLS += qml_carview
qml_carview.files += qml_carview/carview.qml
qml_carview.files += qml_carview/Button.qml
qml_carview.files += qml_carview/MetadataText.qml
qml_carview.files += qml_carview/Playlist.qml
qml_carview.files += qml_carview/Slider.qml
qml_carview.files += qml_carview/SongView.qml
qml_carview.path = $$PKGDATADIR/qml/carview/

INSTALLS += lang
for(TSFILE, TRANSLATIONS) {
    QMFILE = $$replace(TSFILE, ".ts", ".qm")
    lang.files += $$OUT_PWD/$$basename(QMFILE)
}
lang.CONFIG += no_check_exist
lang.path = $$PKGDATADIR/lang
