# This file tells Qt Creator where to look for specific libraries.
# This is needed so that code completion works, and no warnings are shown when
# looking at the code.
#
# Feel free to use the following command so that git stops tracking this file
# git update-index --assume-unchanged external-includepaths.pro
# To make git track it again:
# git update-index --no-assume-unchanged external-includepaths.pro
#
# The following examples should help you figure things out.
# Uncomment the lines as required.

# Under OSX (where john is the username):
#QTCREATORINSTALLPATH = /Users/john/NokiaQtSDK

# Under Linux (where john is the username):
#QTCREATORINSTALLPATH = /home/john/NokiaQtSDK

count(QTCREATORINSTALLPATH, 0) {
    error("You need to set the Qt Creator install path in external-includepaths.pro")
}

!exists($$QTCREATORINSTALLPATH) {
    error("Invalid Qt Creator install path.")
}

INCLUDEPATH += \
    $$QTCREATORINSTALLPATH/Maemo/4.6.2/sysroots/fremantle-arm-sysroot-20.2010.36-2-slim/usr/include/glib-2.0/ \
    $$QTCREATORINSTALLPATH/Maemo/4.6.2/sysroots/fremantle-arm-sysroot-20.2010.36-2-slim/usr/include/mafw-1.0/ \
    $$QTCREATORINSTALLPATH/Maemo/4.6.2/sysroots/fremantle-arm-sysroot-20.2010.36-2-slim/usr/include/gq
# /usr/include/gq is provided by libgq-gconf-dev and libgq-gconf0
# These packages need to be installed on top of your sysrootfs.
# Join #maemo-foss on FreeNode for more information.
