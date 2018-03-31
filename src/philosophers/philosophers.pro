QT -= core gui

TARGET = philosophers
TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++1z
QMAKE_CXXFLAGS += -nostdinc++

INCLUDEPATH += /usr/include/x86_64-linux-gnu/c++/7
INCLUDEPATH += /usr/include/c++/7


OTHER_FILES += \
    philosophers.pri

unix {
    target.path = /usr/lib
    INSTALLS += target
}

include($$PWD/../../lib/cpp_functional_core/cpp_functional_core.pri)
include($$PWD/../../lib/cpp_stm/cpp_stm.pri)

HEADERS += \
    philosophers_stm.h \
    philosophers_types.h
