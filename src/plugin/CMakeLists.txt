set(PluginDir ${CMAKE_CURRENT_SOURCE_DIR})

add_library(pluginTopDir INTERFACE)
target_include_directories(pluginTopDir INTERFACE ${CMAKE_CURRENT_LIST_DIR})
add_library(plugin::plugin ALIAS pluginTopDir)


add_subdirectory(plugin/Heater)
add_subdirectory(plugin/TempMonitor)
