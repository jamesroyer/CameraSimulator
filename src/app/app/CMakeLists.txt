find_package(Threads REQUIRED)

#	app/Info.cpp
#	app/Lens.cpp
add_executable(camera
	Application.cpp
	Base.cpp
	Command.cpp
	Compass.cpp
	McuMisc.cpp
	PluginLoader.cpp
	Registry.cpp
	Status.cpp
	main.cpp
)

set_target_properties(camera PROPERTIES
	POSITION_INDEPENDENT_CODE ON
)

target_include_directories(camera
	PUBLIC
	${AppDir}
)

target_link_libraries(camera
	PRIVATE
	panel
	ncurses
	spdlog
	Threads::Threads
	pthread
	fmt
	mosquitto
	core::core
	plugin::plugin
)
