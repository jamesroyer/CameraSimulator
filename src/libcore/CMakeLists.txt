find_package(Threads REQUIRED)

add_library(core
	core/BaseThread.cpp
	core/Logger.cpp
	core/MqttClient.cpp
	core/Notifier.cpp
	core/Utils.cpp
)

add_library(core::core ALIAS core)

target_include_directories(core
	PUBLIC
	${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(core
	PRIVATE
	panel
	ncurses
	spdlog
	Threads::Threads
	pthread
	fmt
	mosquitto
)
