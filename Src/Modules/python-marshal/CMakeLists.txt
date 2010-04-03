include(../../../LuaPlusBuildTypes.cmake)

set(SRCS )

COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/marshal.lua ${EXECUTABLE_OUTPUT_PATH}/lua/python/marshal.lua SRCS)

add_custom_target(python.marshal DEPENDS ${SRCS})
