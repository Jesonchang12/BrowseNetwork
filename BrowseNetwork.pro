#-------------------------------------------------
#
# Project created by QtCreator 2020-08-25T18:01:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

TARGET = BrowseNetwork
TEMPLATE = app


SOURCES += main.cpp\
        NodeListTable.cpp \
    NetworkSearch.cpp

HEADERS  += NodeListTable.h \
    NetworkSearch.h

FORMS    += NodeListTable.ui
