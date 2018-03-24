#-------------------------------------------------
#
# Project created by QtCreator 2014-11-10T21:19:42
#
#-------------------------------------------------

QT       -= core gui
TARGET = philosophers_console
TEMPLATE = app

CONFIG += c++1z
QMAKE_CXXFLAGS += -nostdinc++

INCLUDEPATH += /usr/include/x86_64-linux-gnu/c++/7
INCLUDEPATH += /usr/include/c++/7

SOURCES += \
    main.cpp

include($$PWD/../../lib/cpp_functional_core/cpp_functional_core.pri)
include($$PWD/../../lib/cpp_stm/cpp_stm.pri)
include($$PWD/../../src/philosophers/philosophers.pri)
