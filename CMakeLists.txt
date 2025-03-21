## Documentation that may be useful:
## https://stackoverflow.com/questions/69405756/problem-using-fetchcontent-declare-together-with-shared-library

cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

function(print)
	foreach(arg IN LISTS ARGV)
		message("=== ** ${arg} = ${${arg}} ** ===")
	endforeach()
endfunction()

project(CamSim
	VERSION 1.0.0
	LANGUAGES CXX
	DESCRIPTION "components loaded at runtime and use messages to communicate"
)

set(CMAKE_CXX_STANDARD 20)

#enable_testing()

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFile)
include(CheckIncludeFiles)
#include(CheckTypeSize)

include(GNUInstallDirs)

set(CMAKE_INSTALL_RPATH "\$ORIGIN")
#endif()

#set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_STANDARD 17)

set(BUILD_SHARED_LIBS ON CACHE BOOL INTERNAL "Build shared libraries")

include(ExternalProject)
include(FetchContent)

if(FALSE)
ExternalProject_Add(SpdLog
	GIT_REPOSITORY https://github.com/gabime/spdlog
	GIT_TAG v1.15.1
	INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/spdlog
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)
endif()

if(TRUE)
ExternalProject_Add(JsonRpcCxx
	GIT_REPOSITORY https://github.com/jsonrpcx/json-rpc-cxx.git
	GIT_TAG v0.3.2
	INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/json-rpc-cxx
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)
endif()

FetchContent_Declare(spdlog_src
	GIT_REPOSITORY https://github.com/gabime/spdlog
	GIT_TAG v1.15.1
	SYSTEM
)

# First, specify all sources using FetchContent_Declare().
FetchContent_Declare(zcm_src
	GIT_REPOSITORY https://github.com/ZeroCM/zcm
#	GIT_TAG v1.1.5
	SYSTEM
)

if(FALSE)
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()


# Second, download all content declared above.
FetchContent_MakeAvailable(
	spdlog_src
	zcm_src
#	googletest
)

FetchContent_GetProperties(spdlog_src)

#add_subdirectory(spdlog_src_SOURCE_DIR)

if(PROJECT_IS_TOPLEVEL)
	install(TARGETS spdlog_src)
endif()

FetchContent_GetProperties(zcm_src)

# Install the zcm binaries where the project can find them.
set(zcm_prefix ${CMAKE_BINARY_DIR}/zcm)

# TODO: Determine if this needs to be done before project() is called to ensure
# that "zcm-gen" is built as early as possible to be able to generate the ...
# files.
add_custom_command(
	OUTPUT
		${zcm_prefix}/bin/zcm-gen
		# ... do I need to list header files? ...
	COMMAND ${zcm_src_SOURCE_DIR}/waf --top=${zcm_src_SOURCE_DIR} --prefix=${zcm_prefix} configure --use-ipc --use-zmq --use-clang
	COMMAND ${zcm_src_SOURCE_DIR}/waf --top=${zcm_src_SOURCE_DIR} --prefix=${zcm_prefix} build
	COMMAND ${zcm_src_SOURCE_DIR}/waf --top=${zcm_src_SOURCE_DIR} --prefix=${zcm_prefix} install
)
add_custom_target(zcm ALL
	DEPENDS ${zcm_prefix}/bin/zcm-gen
)

# Include spdlog as an imported target.
#set(SPDLOG_BUILD_SHARED ON CACHE BOOL "Create libspdlog.so")
#add_subdirectory(libs/spdlog)

# location for building binary files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
#add_subdirectory(tests)

#include(GoogleTest)
#gtest_discover_tests(test_file)
