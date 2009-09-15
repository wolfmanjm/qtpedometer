TEMPLATE=app
TARGET=qtpedometer

CONFIG+=qtopia

# I18n info
STRING_LANGUAGE=en_US
LANGUAGES=en_US

# Package info
pkg [
    name=qtpedometer
    desc="A GPS based pedometer"
    license=GPL
    version=1.0
    maintainer="Jim Morris <morris@wolfman.com>"
]

# Input files
FORMS=\
    qtpedometer.ui

HEADERS=\
    qtpedometer.h

SOURCES=\
    main.cpp\
    qtpedometer.cpp

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

