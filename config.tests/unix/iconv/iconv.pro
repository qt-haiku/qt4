SOURCES = iconv.cpp
CONFIG -= qt dylib app_bundle
mac|win32-g++*|qnx-*-qcc|haiku:LIBS += -liconv