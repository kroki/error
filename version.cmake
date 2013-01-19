include(FindGit)

if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(COMMAND ${GIT_EXECUTABLE} describe --dirty
                  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                  OUTPUT_VARIABLE TAG
                  OUTPUT_STRIP_TRAILING_WHITESPACE
                  ERROR_QUIET)
  if(NOT "${TAG}" STREQUAL "")
    string(REGEX MATCHALL "^([^-]+)(-(.+))?$" UNUSED "${TAG}")
    set(CPACK_PACKAGE_VERSION "${CMAKE_MATCH_1}")
    if (NOT "${CMAKE_MATCH_3}" STREQUAL "")
      if(NOT "${CMAKE_MATCH_3}" STREQUAL "dirty")
        string(REGEX MATCHALL "^([0-9]+)-(.+)$" UNUSED "${CMAKE_MATCH_3}")
        math(EXPR CMAKE_MATCH_1 "${CMAKE_MATCH_1} + 1")
        string(REPLACE "-" "_" CMAKE_MATCH_2 "${CMAKE_MATCH_2}")
        set(CPACK_RPM_PACKAGE_RELEASE "${CMAKE_MATCH_1}_${CMAKE_MATCH_2}")
      else()
        set(CPACK_RPM_PACKAGE_RELEASE "1_${CMAKE_MATCH_3}")
      endif()
    else()
      set(CPACK_RPM_PACKAGE_RELEASE 1)
    endif()
  else()
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    OUTPUT_VARIABLE COUNT
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --dirty --always
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    OUTPUT_VARIABLE ID
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET)
    set(CPACK_PACKAGE_VERSION 0.0)
    if(NOT "${COUNT}" STREQUAL "")
      math(EXPR COUNT "${COUNT} + 1")
      string(REPLACE "-" "_" CPACK_RPM_PACKAGE_RELEASE "${COUNT}_g${ID}")
    else()
      set(CPACK_RPM_PACKAGE_RELEASE "1_dirty")
    endif()
  endif()
else()
  set(CPACK_PACKAGE_VERSION 0.0)
  set(CPACK_RPM_PACKAGE_RELEASE "1_dirty")
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.gen.in"
               "${CMAKE_BINARY_DIR}/version.gen"
               @ONLY)
