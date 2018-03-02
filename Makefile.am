ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -Wall
AM_CFLAGS =
AM_CXXFLAGS =
AM_LDFLAGS =

check_PROGRAMS = \
				 test1 \
				 test2

test1_SOURCES = ./tests/test1.cpp
test1_LDFLAGS = -all-static

test2_SOURCES = ./tests/test2.cpp
test2_LDFLAGS = -all-static

TESTS = $(check_PROGRAMS)
