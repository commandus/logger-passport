SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4

#
# https://ask-dev.ru/info/1247790/understanding-gcc-5s-glibcxxusecxx11abi-or-the-new-abi
#-D_GLIBCXX_USE_CXX11_ABI=0
# -I../,,/third_party for make dist little hack
#
COMMON_C_FLAGS = -Ithird_party -I../../third_party -DDEBUG_PRINT -O2
AM_CXXFLAGS= -std=c++11 $(COMMON_C_FLAGS)
AM_CPPFLAGS = $(COMMON_C_FLAGS)

HDRS = \
	platform.h errlist.h logger-passport.h logger-passport-collection.h \
	list-file.h \
	third_party/argtable3/argtable3.h

#
# Binaries
#
bin_PROGRAMS = logger-passport-print

LIBLOGGERPASSPORTSRC = \
	logger-passport.cpp logger-passport-collection.cpp list-file.cpp \
	errlist.cpp

#
#	Static library
#
lib_LIBRARIES = liblogger-passport.a
liblogger_passport_a_SOURCES = $(LIBLOGGERPASSPORTSRC)
# $(HDRS)
liblogger_passport_a_CXXFLAGS = -std=c++11 $(COMMON_C_FLAGS) -fPIC

L_LOGGER_PASSPORT = -L. -llogger-passport

# Static library used for buil binaries, so build library first.
$(bin_PROGRAMS): $(lib_LIBRARIES)

# EXTRA_DIST = $(HDRS)
include_HEADERS = $(HDRS)
# nobase_dist_include_HEADERS =

logger_passport_print_SOURCES = \
	logger-passport-print.cpp \
	third_party/argtable3/argtable3.c
logger_passport_print_LDADD = \
	$(L_LOGGER_PASSPORT)
logger_passport_print_CPPFLAGS = 

#
# Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
   autogen.sh CMakeLists.txt CODE_OF_CONDUCT.md CONTRIBUTING.md COPYING HISTORY LICENSE README.md TODO \
   third_party/argtable3/README

#
# Tests
#
test_compress_decompress_SOURCES = \
	tests/test-compress-decompress.cpp
test_compress_decompress_LDADD = $(L_LOGGER_PASSPORT)

test_temperature_SOURCES = \
	tests/test-temperature.c
test_temperature_LDADD = $(L_LOGGER_PASSPORT)

test_sql_SOURCES = \
	tests/test-sql.cpp
test_sql_LDADD = $(L_LOGGER_PASSPORT)

check_PROGRAMS = \
	test-compress-decompress test-temperature test-sql

TESTS = $(check_PROGRAMS)
