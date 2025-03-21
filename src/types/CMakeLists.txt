# XXX: Is this the correct way to generate header files?
# XXX: What causes them to be created? Where are the dependencies added?


function(add_zcm_type name)
	set(zcm_gen ${zcm_prefix}/bin/zcm-gen)
	#message("JR === ${zcm_gen} ===")
	set(incbasedir ${CMAKE_BINARY_DIR}/include/types)
	set(incdir ${incbasedir}/Demo)
	add_custom_command(
		OUTPUT ${incdir}/${name}.hpp
		COMMAND ${zcm_gen} --cpp --cpp-hpath ${incbasedir} ${CMAKE_CURRENT_SOURCE_DIR}/${name}.zcm
		DEPENDS ${name}.zcm)
#		MAIN_DEPENDENCY ${name}.zcm

	set(gen_types ${gen_types} ${incdir}/${name}.hpp PARENT_SCOPE)

#	add_custom_target(generate_${name} DEPENDS ${incdir}/${name}.hpp)
endfunction()

add_zcm_type(heater_t)
add_zcm_type(power_level_t)
add_zcm_type(shutdown_t)
add_zcm_type(start_t)
add_zcm_type(temperature_t)

# This target is added as a dependency to plugins to ensure these are
# generated.
add_custom_target(generate_zcm_types DEPENDS ${gen_types})
add_dependencies(generate_zcm_types zcm)
