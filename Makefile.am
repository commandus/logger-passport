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

RAPID_HDRS = \
	third_party/rapidjson/allocators.h       third_party/rapidjson/encodings.h        third_party/rapidjson/fwd.h             third_party/rapidjson/memorystream.h    third_party/rapidjson/prettywriter.h   third_party/rapidjson/schema.h  \
	third_party/rapidjson/writer.h           third_party/rapidjson/document.h         third_party/rapidjson/filereadstream.h  third_party/rapidjson/istreamwrapper.h  third_party/rapidjson/ostreamwrapper.h third_party/rapidjson/rapidjson.h \
	third_party/rapidjson/stream.h           third_party/rapidjson/encodedstream.h    third_party/rapidjson/filewritestream.h third_party/rapidjson/memorybuffer.h    third_party/rapidjson/pointer.h        third_party/rapidjson/reader.h \
	third_party/rapidjson/stringbuffer.h     third_party/rapidjson/error/en.h         third_party/rapidjson/error/error.h     third_party/rapidjson/internal/biginteger.h third_party/rapidjson/internal/dtoa.h  third_party/rapidjson/internal/itoa.h \
	third_party/rapidjson/internal/pow10.h   third_party/rapidjson/internal/stack.h   third_party/rapidjson/internal/strtod.h third_party/rapidjson/internal/diyfp.h  third_party/rapidjson/internal/ieee754.h third_party/rapidjson/internal/meta.h \
	third_party/rapidjson/internal/regex.h   third_party/rapidjson/internal/strfunc.h third_party/rapidjson/internal/swap.h \
	third_party/rapidjson/msinttypes/inttypes.h third_party/rapidjson/msinttypes/stdint.h

HDRS = \
	platform.h errlist.h logger-passport.h logger-plume-collection.h list-file.h \
	third_party/argtable3/argtable3.h \
	third_party/filewatch.hpp \
	$(RAPID_HDRS)

#
# Binaries
#
bin_PROGRAMS = logger-passport-print

LIBLOGGERPASSPORTSRC = \
	logger-passport.cpp logger-plume-collection.cpp list-file.cpp \
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
test_passport_SOURCES = \
	tests/test-passport.cpp
test_passport_LDADD = $(L_LOGGER_PASSPORT)

check_PROGRAMS = \
	test-passport

TESTS = $(check_PROGRAMS)
