QT       += core gui sql network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MedicalSystem
TEMPLATE = app

SOURCES += main.cpp \
           MainWindow.cpp \
           DatabaseManager.cpp \
           StatsWorker.cpp \
           NetworkSync.cpp

HEADERS += MainWindow.h \
           DatabaseManager.h \
           StatsWorker.h \
           NetworkSync.h

FORMS += MainWindow.ui
