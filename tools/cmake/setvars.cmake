include_guard()

# This guy gets picked up by Catch2 tests as well as core estd itself

get_filename_component(TOOLS_DIR ${CMAKE_CURRENT_LIST_DIR} ABSOLUTE)
get_filename_component(ROOT_DIR ${TOOLS_DIR}/../.. ABSOLUTE)
set(ESTD_DIR ${ROOT_DIR}/src)

include(${TOOLS_DIR}/CPM.cmake)
include(${TOOLS_DIR}/fetchcontent.cmake)
include(${TOOLS_DIR}/options.cmake)
include(${TOOLS_DIR}/compiler.cmake)
