include(CMakeParseArguments)

function(AddFont appsources)
  set(options)
  set(oneValueArgs OUTFILE_BASENAME)
  set(multiValueArgs FONT)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT ARG_FONT)
    message(FATAL_ERROR "No FONT argument given to AddFonts")
  endif ()
  if (ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unexpected arguments to AddFonts: ${ARG_UNPARSED_ARGUMENTS}")
  endif ()

  foreach (font ${ARG_FONT})
    get_filename_component(font_full ${font} ABSOLUTE)
    get_filename_component(font_name ${font_full} NAME_WE)

    set(font_full_output ${CMAKE_CURRENT_BINARY_DIR}/${font_name}.ttf)
    configure_file(${font_full} ${font_full_output} COPYONLY)
    set(${appsources} "${${appsources}};${font_full_output}" PARENT_SCOPE)
    set_source_files_properties(${font_full_output} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  endforeach ()

  return()
endfunction()