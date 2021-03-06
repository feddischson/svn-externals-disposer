TEMPLATE = app
TARGET = test_svn_externals_disposer
INCLUDEPATH += ./include ./test
DEPENDPATH += . include src test

QMAKE_CXXFLAGS += -Werror # set -werror
QMAKE_CXXFLAGS += -Wextra # set -wextra
CONFIG += warn_on         # 
CONFIG += c++11           # set -std=c++0x
CONFIG += testcase        # enable test-cases for make
CONFIG += debug


QT          += widgets
QT          += xml
QT          += core
QT          += testlib

DEFINES += TESTING

HEADERS     += include/const.h
HEADERS     += include/external.h
HEADERS     += include/data_model.h
HEADERS     += include/external_command.h
HEADERS     += include/browser_item.h
HEADERS     += include/browser_model.h
HEADERS     += test/test_external.h
HEADERS     += test/test_data_model.h
HEADERS     += test/test_browser_item.h
HEADERS     += test/test_browser_model.h
SOURCES     += src/external.cpp
SOURCES     += src/data_model.cpp
SOURCES     += src/external_command.cpp
SOURCES     += src/browser_item.cpp
SOURCES     += src/browser_model.cpp
SOURCES     += test/test_external.cpp
SOURCES     += test/test_data_model.cpp
SOURCES     += test/test_browser_item.cpp
SOURCES     += test/test_browser_model.cpp
SOURCES     += test/main.cpp

VERSION = "1.1.0"
DEFINES += _APP_VERSION=\\\"$$VERSION\\\"

