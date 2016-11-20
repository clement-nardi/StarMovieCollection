#-------------------------------------------------
#
# Project created by QtCreator 2016-11-05T14:49:19
#
#-------------------------------------------------

DESTDIR=bin

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StarMovieCollection
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    tmdbapi.cpp \
    persistentcache.cpp \
    videocollectionmodel.cpp \
    videocollectiondelegate.cpp \
    videofile.cpp \
    video.cpp \
    romanconverter.cpp \
    patterns.cpp


HEADERS  += mainwindow.h \
    tmdbapi.h \
    persistentcache.h \
    video.h \
    videocollectiondelegate.h \
    videocollectionmodel.h \
    videofile.h \
    romanconverter.h \
    patterns.h

FORMS    += mainwindow.ui


DISTFILES += \
    $$PWD/titlePatterns.json \
    $$PWD/directorsAndActorsNames.txt
#Runtime dependencies
win32 {
    #...
    EXTRA_BINFILES += \
        C:\Qt\Qt5.7.0\Tools\mingw530_32\opt\bin\ssleay32.dll \
        C:\Qt\Qt5.7.0\Tools\mingw530_32\opt\bin\libeay32.dll
    EXTRA_BINFILES_WIN = $${EXTRA_BINFILES}
    EXTRA_BINFILES_WIN ~= s,/,\\,g
    DESTDIR_WIN = $${DESTDIR}
    DESTDIR_WIN ~= s,/,\\,g
    DISTFILES_WIN = $${DISTFILES}
    DISTFILES_WIN ~= s,/,\\,g

    for(FILE,EXTRA_BINFILES_WIN){
        QMAKE_POST_LINK +=$$quote(cmd /c copy /y $${FILE} $${DESTDIR_WIN}$$escape_expand(\n\t))
    }
    FILE_WIN = $${PWD}/../Application/titlePatterns.json
    FILE_WIN ~= s,/,\\,g
    titlePatterns.commands = $$quote(cmd /c copy /y $${FILE_WIN} $${DESTDIR_WIN}$$escape_expand(\n\t))
    FILE_WIN = $${PWD}/../Application/directorsAndActorsNames.txt
    FILE_WIN ~= s,/,\\,g
    names.commands = $$quote(cmd /c copy /y $${FILE_WIN} $${DESTDIR_WIN}$$escape_expand(\n\t))
}

titlePatterns.target = $${DESTDIR}/titlePatterns.json
titlePatterns.depends = $${PWD}/../Application/titlePatterns.json
names.target = $${DESTDIR}/directorsAndActorsNames.txt
names.depends = $${PWD}/../Application/directorsAndActorsNames.txt

QMAKE_EXTRA_TARGETS += titlePatterns names
PRE_TARGETDEPS += $${DESTDIR}/titlePatterns.json $${DESTDIR}/directorsAndActorsNames.txt
