include(CMakeParseArguments)

function(AddPNG appsources)
  set(options)
  set(oneValueArgs OUTFILE_BASENAME)
  set(multiValueArgs PNG)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT ARG_PNG)
    message(FATAL_ERROR "No PNG argument given to AddPNG")
  endif ()
  if (ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unexpected arguments to AddPNG: ${ARG_UNPARSED_ARGUMENTS}")
  endif ()

  foreach (png ${ARG_PNG})
    get_filename_component(png_full ${png} ABSOLUTE)
    get_filename_component(png_name ${png_full} NAME_WE)

    set(png_full_output ${CMAKE_CURRENT_BINARY_DIR}/${png_name}.png)
    configure_file(${png_full} ${png_full_output} COPYONLY)
    set(${appsources} "${${appsources}};${png_full_output}" PARENT_SCOPE)
    set_source_files_properties(${png_full_output} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

  endforeach ()

  return()
endfunction()