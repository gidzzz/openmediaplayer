QT += core gui dbus declarative opengl network maemo5

TARGET = openmediaplayer
TEMPLATE = app

INCLUDEPATH += ../lyrics

DEFINES += MAFW_WORKAROUNDS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    musicwindow.cpp \
    nowplayingwindow.cpp \
    videoswindow.cpp \
    internetradiowindow.cpp \
    sharedialog.cpp \
    ringtonedialog.cpp \
    videonowplayingwindow.cpp \
    cqgraphicsview.cpp \
    nowplayingindicator.cpp \
    radionowplayingwindow.cpp \
    singlealbumview.cpp \
    singleartistview.cpp \
    settingsdialog.cpp \
    qmlview.cpp \
    singlegenreview.cpp \
    singleplaylistview.cpp \
    aboutwindow.cpp \
    coverpicker.cpp \
    lyricseditdialog.cpp \
    lyricssearchdialog.cpp \
    mediaart.cpp \
    missioncontrol.cpp \
    playbackmanager.cpp \
    metadatawatcher.cpp \
    sleeper.cpp \
    playlistquerymanager.cpp \
    pluginscontrol.cpp \
    pluginswindow.cpp \
    upnpcontrol.cpp \
    upnpview.cpp \
    rotator.cpp \
    playlistpicker.cpp \
    sleeperdialog.cpp \
    bookmarkdialog.cpp \
    lyricsmanager.cpp \
    lyricsprovidersdialog.cpp \
    basewindow.cpp \
    browserwindow.cpp \
    opendialog.cpp \
    currentplaylistmanager.cpp \
    maemo5deviceevents.cpp \
    fmtxinterface.cpp \
    fmtxdialog.cpp \
    frequencypickselector.cpp \
    frequencypickdialog.cpp \
    delegates/artistlistitemdelegate.cpp \
    delegates/maindelegate.cpp \
    delegates/mediawithicondelegate.cpp \
    delegates/playlistdelegate.cpp \
    delegates/providerlistitemdelegate.cpp \
    delegates/shufflebuttondelegate.cpp \
    delegates/singlealbumviewdelegate.cpp \
    delegates/songlistitemdelegate.cpp \
    delegates/thumbnailitemdelegate.cpp \
    mafw/currentplaylistadapter.cpp \
    mafw/mafwutils.cpp \
    mafw/mafwplaylistadapter.cpp \
    mafw/mafwplaylistmanageradapter.cpp \
    mafw/mafwregistryadapter.cpp \
    mafw/mafwrendereradapter.cpp \
    mafw/mafwsourceadapter.cpp

HEADERS += \
    mainwindow.h \
    musicwindow.h \
    nowplayingwindow.h \
    videoswindow.h \
    internetradiowindow.h \
    sharedialog.h \
    ringtonedialog.h \
    videonowplayingwindow.h \
    mirror.h \
    cqgraphicsview.h \
    nowplayingindicator.h \
    includes.h \
    radionowplayingwindow.h \
    singlealbumview.h \
    singleartistview.h \
    settingsdialog.h \
    qmlview.h \
    singlegenreview.h \
    singleplaylistview.h \
    aboutwindow.h \
    coverpicker.h \
    texteditautoresizer.h \
    lyricseditdialog.h \
    lyricssearchdialog.h \
    mediaart.h \
    missioncontrol.h \
    playbackmanager.h \
    metadatawatcher.h \
    sleeper.h \
    playlistquerymanager.h \
    pluginscontrol.h \
    pluginswindow.h \
    upnpcontrol.h \
    upnpview.h \
    rotator.h \
    playlistpicker.h \
    sleeperdialog.h \
    bookmarkdialog.h \
    ../lyrics/abstractlyricsprovider.h \
    lyricsmanager.h \
    lyricsprovidersdialog.h \
    headerawareproxymodel.h \
    confirmdialog.h \
    fastlistview.h \
    basewindow.h \
    browserwindow.h \
    kbmenu.h \
    opendialog.h \
    currentplaylistmanager.h \
    maemo5deviceevents.h \
    fmtxinterface.h \
    fmtxdialog.h \
    frequencypickselector.h \
    frequencypickdialog.h \
    delegates/artistlistitemdelegate.h \
    delegates/maindelegate.h \
    delegates/mediawithicondelegate.h \
    delegates/playlistdelegate.h \
    delegates/providerlistitemdelegate.h \
    delegates/shufflebuttondelegate.h \
    delegates/singlealbumviewdelegate.h \
    delegates/songlistitemdelegate.h \
    delegates/thumbnailitemdelegate.h \
    mafw/currentplaylistadapter.h \
    mafw/mafwutils.h \
    mafw/mafwplaylistadapter.h \
    mafw/mafwplaylistmanageradapter.h \
    mafw/mafwregistryadapter.h \
    mafw/mafwrendereradapter.h \
    mafw/mafwsourceadapter.h

FORMS += \
    mainwindow.ui \
    musicwindow.ui \
    nowplayingwindow.ui \
    sharedialog.ui \
    fmtxdialog.ui \
    frequencypickdialog.ui \
    videonowplayingwindow.ui \
    nowplayingindicator.ui \
    radionowplayingwindow.ui \
    settingsdialog.ui \
    qmlview.ui \
    aboutwindow.ui \
    coverpicker.ui \
    lyricseditdialog.ui \
    lyricssearchdialog.ui \
    playlistpicker.ui \
    sleeperdialog.ui \
    bookmarkdialog.ui \
    lyricsprovidersdialog.ui \
    browserwindow.ui \
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

LIBS += -lhildonthumbnail -lX11
CONFIG += link_pkgconfig
PKGCONFIG += mafw mafw-shared glib-2.0 gnome-vfs-2.0 libplayback-1 dbus-1 gtk+-2.0

TRANSLATIONS = \
    lang/openmediaplayer.ar_SA.ts \
    lang/openmediaplayer.bg.ts \
    lang/openmediaplayer.cs.ts \
    lang/openmediaplayer.de.ts \
    lang/openmediaplayer.en.ts \
    lang/openmediaplayer.es.ts \
    lang/openmediaplayer.es_AR.ts \
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
    lang/openmediaplayer.sl_SI.ts \
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
