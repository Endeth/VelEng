MACRO(ADDFILES result dir)
	FILE(GLOB_RECURSE source_list
		"${dir}/*.h"
		"${dir}/*.cpp"
	)

	LIST(FILTER source_list EXCLUDE REGEX "ex\\_")

	foreach(source IN LISTS source_list)
		if (IS_ABSOLUTE "${source}")
			file(RELATIVE_PATH source_rel "${dir}" "${source}")
		else()
			set(source_rel "${source}")
		endif()

		get_filename_component(source_path "${source_rel}" PATH)
		string(REPLACE "/" "\\" source_path_msvc "${source_path}")
		source_group("${source_path_msvc}" FILES "${source}")
	endforeach()

	SET(${result} ${source_list})

ENDMACRO()