add_library(PluginTempMonitor MODULE
	TempMonitorTask.cpp
	PluginTempMonitor.cpp
)
#	${CMAKE_BINARY_DIR}/include/types/demo/temperature_t.hpp)

set_target_properties(PluginTempMonitor PROPERTIES
	POSITION_INDEPENDENT_CODE ON
)

# If we have compiler requirements for this library, list them here.
target_compile_features(PluginTempMonitor
	PUBLIC
	cxx_strong_enums
	cxx_auto_type
	PRIVATE
	cxx_lambdas
	cxx_range_for
	cxx_variadic_templates
)

cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH PARENT_DIR)

target_include_directories(PluginTempMonitor
	PUBLIC
	${CMAKE_BINARY_DIR}/include
	${CMAKE_BINARY_DIR}/zcm/include
	${PluginDir}
)

print(PARENT_DIR CMAKE_CURRENT_LIST_DIR CMAKE_BINARY_DIR)
if(FALSE)
# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to the consumer's
# build paths.
	target_include_directories(PluginTempMonitor
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
	${CMAKE_BINARY_DIR}/zcm/include
	PRIVATE
	src
	../include
	${CMAKE_BINARY_DIR}/include
)
endif()

# Depend on a library that we defined in the top-level file.
target_link_libraries(PluginTempMonitor
	PRIVATE
	spdlog
	core::core
)

## 'make install' to the correct locations (provided by GNUInstallDirs).
#install(TARGETS PluginTempMonitor EXPORT MyLibraryConfig
#	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}) # This is for Windows
#install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

## This makes the project importable from the install directory.
## Put config file in per-project dir (name MUST match), can also
## just go into 'cmake'.
#install(EXPORT MyLibraryConfig DESTINATION share/MyLibrary/cmake)

## This makes the project importable from the build directory.
#export(TARGETS PluginTempMonitor FILE MyLibraryConfig.cmake)

if(FALSE)
# Every library has unit tests, of course.
add_executable(testlibheater
	test/testlib.cpp)

target_link_libraries(testlibheater
	PluginTempMonitor)

add_test(testlibheater testlibheater)
endif()

install(
	TARGETS PluginTempMonitor
	LIBRARY DESTINATION lib
	COMPONENT library
)
