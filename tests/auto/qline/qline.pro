load(qttest_p4)
QT -= gui
SOURCES += tst_qline.cpp
unix:!mac:!symbian:!vxworks:!haiku:LIBS+=-lm


CONFIG += parallel_test
