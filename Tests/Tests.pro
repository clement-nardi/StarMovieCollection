
DESTDIR = bin

QT       += core testlib network
CONFIG += testcase

SOURCES += tests.cpp \
           ../Application/tmdbapi.cpp \
           ../Application/persistentcache.cpp

HEADERS  += tests.h \
            ../Application/tmdbapi.h \
            ../Application/persistentcache.h

INCLUDEPATH += ../Application


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
    for(FILE,EXTRA_BINFILES_WIN){
                QMAKE_POST_LINK +=$$quote(cmd /c copy /y $${FILE} $${DESTDIR_WIN}$$escape_expand(\n\t))
    }
}
