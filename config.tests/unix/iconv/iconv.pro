SOURCES = iconv.cpp
CONFIG -= qt dylib app_bundle
mac|win32-g++*|haiku:LIBS += -liconv
