cmake_minimum_required(VERSION 3.9)
set(PROJECT_NAME "logger-passport")
project(${PROJECT_NAME} VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)

set(LIB_LOGGER_PASSPORT_SRC
	logger-passport.cpp logger-passport-collection.cpp 
	list-file.cpp errlist.cpp
)
set(SRC_LOGGER_PASSPORT_PRINT
	logger-passport-print.cpp
	third_party/argtable3/argtable3.c
)

include_directories("third_party")

add_library(logger-passport STATIC
	${LIB_LOGGER_PASSPORT_SRC}
)

add_executable(logger-passport-print ${SRC_LOGGER_PASSPORT_PRINT})
target_link_libraries(logger-passport-print PRIVATE
	logger-passport
)

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/loger-passport-print DESTINATION cmake)
INSTALL(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION doc)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "1")
include(CPack)
