QT += core gui opengl

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TARGET = hbao
TEMPLATE = app

CONFIG += c++14
CONFIG(release, release|debug):QMAKE_CXXFLAGS += -Wall -O2

CONFIG(release, release|debug):DESTDIR = ../build/release/
CONFIG(release, release|debug):OBJECTS_DIR = ../build/release/
CONFIG(release, release|debug):MOC_DIR = ../build/release/
CONFIG(release, release|debug):UI_DIR = ../build/release/

CONFIG(debug, release|debug):DESTDIR = ../build/debug/
CONFIG(debug, release|debug):OBJECTS_DIR = ../build/debug/
CONFIG(debug, release|debug):MOC_DIR = ../build/debug/
CONFIG(debug, release|debug):UI_DIR = ../build/debug/

INCLUDEPATH += /usr/include/eigen3/

LIBS += -lGLEW

SOURCES += \
    triangle_mesh.cc \
    mesh_io.cc \
    main.cc \
    main_window.cc \
    glwidget.cc \
    camera.cc

HEADERS  += \
    triangle_mesh.h \
    mesh_io.h \
    main_window.h \
    glwidget.h \
    camera.h

FORMS    += \
    main_window.ui

DISTFILES += \
    ../res/shaders/g.frag \
    ../res/shaders/g.vert \
    ../res/shaders/blur.vert \
    ../res/shaders/blur.frag \
    ../res/shaders/hbao.vert \
    ../res/shaders/hbao.frag \
    ../res/shaders/depth.vert \
    ../res/shaders/depth.frag \
    ../res/shaders/normal.vert \
    ../res/shaders/normal.frag

