win32:      CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../src/philosophers/release/ -lphilosophers
else:win32: CONFIG(debug, debug|release):   LIBS += -L$$OUT_PWD/../../src/philosophers/debug/   -lphilosophers
else:unix:                                  LIBS += -L$$OUT_PWD/../../src/philosophers/         -lphilosophers

INCLUDEPATH += $$PWD/../../src/philosophers
DEPENDPATH  += $$PWD/../../src/philosophers

win32-g++:             CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../src/philosophers/release/libphilosophers.a
else:win32-g++:        CONFIG(debug, debug|release):   PRE_TARGETDEPS += $$OUT_PWD/../../src/philosophers/debug/libphilosophers.a
else:win32:!win32-g++: CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../src/philosophers/release/philosophers.lib
else:win32:!win32-g++: CONFIG(debug, debug|release):   PRE_TARGETDEPS += $$OUT_PWD/../../src/philosophers/debug/philosophers.lib
else:unix:                                             PRE_TARGETDEPS += $$OUT_PWD/../../src/philosophers/libphilosophers.a

