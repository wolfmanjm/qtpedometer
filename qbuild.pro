TEMPLATE=app
TARGET=qtpedometer

CONFIG+=qtopia
QTOPIA*=whereabouts
DEFINES+=QT_NO_DEBUG_OUTPUT

# I18n info
STRING_LANGUAGE=en_US
LANGUAGES=en_US

# Package info
pkg [
    name=qtpedometer
    desc="A GPS based pedometer"
    license=GPL
    version=1.1
    maintainer="Jim Morris <morris@wolfman.com>"
]

# Input files
FORMS=\
    qtpedometer.ui\
    settings.ui

HEADERS=\
    qtpedometer.h\
    compass.h

SOURCES=\
    main.cpp\
    qtpedometer.cpp\
    compass.cpp

# Install rules
target [
    hint=sxe
    domain=untrusted
]

desktop [
    hint=desktop
    files=qtpedometer.desktop
    path=/apps/Applications
]

# Install some pictures.
pics [
    hint=pics
    files=pics/*
    path=/pics/qtpedometer
]
