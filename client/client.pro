QT       += core gui
QT += network
QT += core

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chatdialog.cpp \
    home.cpp \
    main.cpp \
    client.cpp \
    receivefiledialog.cpp \
    registerdialog.cpp \
    sendfiledialog.cpp \
    userinfo.cpp

HEADERS += \
    chatdialog.h \
    client.h \
    home.h \
    receivefiledialog.h \
    registerdialog.h \
    sendfiledialog.h \
    userinfo.h

FORMS += \
    chatdialog.ui \
    client.ui \
    home.ui \
    receivefiledialog.ui \
    registerdialog.ui \
    sendfiledialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Client.qrc
