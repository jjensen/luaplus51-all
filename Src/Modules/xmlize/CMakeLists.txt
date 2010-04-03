include(../../../LuaPlusBuildTypes.cmake)

set(SRCS )

COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/xmlize.lua ${EXECUTABLE_OUTPUT_PATH}/lua/xmlize.lua SRCS)

add_custom_target(xmlize DEPENDS ${SRCS})
