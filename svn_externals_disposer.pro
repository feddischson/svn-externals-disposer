TEMPLATE = app
TARGET = svn_externals_disposer
INCLUDEPATH += ./include
DEPENDPATH += . forms include src


QMAKE_CXXFLAGS += -Werror # set -werror
QMAKE_CXXFLAGS += -Wextra # set -wextra
CONFIG += warn_on         # 
CONFIG += c++11           # set -std=c++0x

QT          += widgets
QT          += xml
QT          += core

FORMS += forms/MainWindow.ui
FORMS += forms/LogDialog.ui
FORMS += forms/ExternalsDialog.ui
FORMS += forms/BrowserDialog.ui

HEADERS     += include/const.h
HEADERS     += include/mainwindow.h
HEADERS     += include/data_model.h
HEADERS     += include/external.h
HEADERS     += include/tree_view.h
HEADERS     += include/table_view.h
HEADERS     += include/external_command.h
HEADERS     += include/log_dialog.h
HEADERS     += include/login_dialog.h
HEADERS     += include/externals_dialog.h
HEADERS     += include/filter.h
HEADERS     += include/browser_dialog.h
HEADERS     += include/browser_item.h
HEADERS     += include/browser_model.h
SOURCES     += src/main.cpp
SOURCES     += src/mainwindow.cpp
SOURCES     += src/data_model.cpp
SOURCES     += src/external.cpp
SOURCES     += src/tree_view.cpp
SOURCES     += src/table_view.cpp
SOURCES     += src/external_command.cpp
SOURCES     += src/log_dialog.cpp
SOURCES     += src/login_dialog.cpp
SOURCES     += src/externals_dialog.cpp
SOURCES     += src/filter.cpp
SOURCES     += src/browser_dialog.cpp
SOURCES     += src/browser_item.cpp
SOURCES     += src/browser_model.cpp

VERSION = "1.2.0"
DEFINES += _APP_VERSION=\\\"$$VERSION\\\"

target.path = /usr/local/bin
INSTALLS += target
