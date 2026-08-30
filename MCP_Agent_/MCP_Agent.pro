QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += nostrip
macx: QMAKE_POST_LINK += codesign --force --deep -s - $$shell_quote($$OUT_PWD/$${TARGET}.app) $$escape_expand(\\n\\t)

DEFINES += APP_SRC_DIR=\\\"$$PWD\\\"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    config.cpp \
    main.cpp \
    mainwindow.cpp \
    mcp.cpp \
    phonenumber.cpp \
    requests.cpp \
    template_agents.cpp \
    tgbot.cpp

HEADERS += \
    config.h \
    mainwindow.h \
    mcp.h \
    phonenumber.h \
    requests.h \
    template_agents.h \
    tgbot.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    prompt.md
