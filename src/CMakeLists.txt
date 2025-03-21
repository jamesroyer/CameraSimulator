# Generate header files from .zcm files.
add_subdirectory(types)

# Include spdlog as an imported target
set(SPDLOG_BUILD_SHARED ON CACHE BOOL "Create libspdlog.so")
#add_subdirectory(libs/spdlog)

set(TopDir ${CMAKE_CURRENT_SOURCE_DIR})
# Targets that we develop here.
add_subdirectory(libcore)
add_subdirectory(plugin)
add_subdirectory(app)

# Generated files need to be built before attempting to compile the plugins.
add_dependencies(PluginHeater generate_zcm_types)
