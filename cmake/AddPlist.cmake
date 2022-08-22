include(CMakeParseArguments)

function(AddPlist appsources)
  set(options)
  set(oneValueArgs OUTFILE_BASENAME)
  set(multiValueArgs PLIST)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT ARG_PLIST)
    message(FATAL_ERROR "No PLIST argument given to AddPlist")
  endif ()
  if (ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unexpected arguments to AddPlist: ${ARG_UNPARSED_ARGUMENTS}")
  endif ()

  foreach (plist ${ARG_PLIST})
    get_filename_component(plist_full ${plist} ABSOLUTE)
    get_filename_component(plist_name ${plist_full} NAME_WE)

    set(plist_full_output ${CMAKE_CURRENT_BINARY_DIR}/${plist_name}.plist)
    configure_file(${plist_full} ${plist_full_output} COPYONLY)
    set(${appsources} "${${appsources}};${plist_full_output}" PARENT_SCOPE)
    set(MACOSX_BUNDLE_INFO_PLIST ${plist_name}.plist PARENT_SCOPE)
  endforeach ()

  return()
endfunction()