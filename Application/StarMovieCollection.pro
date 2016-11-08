#-------------------------------------------------
#
# Project created by QtCreator 2016-11-05T14:49:19
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StarMovieCollection
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    tmdbapi.cpp \
    persistentcache.cpp


HEADERS  += mainwindow.h \
    tmdbapi.h \
    persistentcache.h

FORMS    += mainwindow.ui
