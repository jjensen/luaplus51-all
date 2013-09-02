# ---------------------------------------------------------------------------
# CMake build file
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# Project name, sets ${wxStEdit_SOURCE_DIR} and ${wxStEdit_BINARY_DIR}

project( wxStEdit )

# ===========================================================================
# General settings for CMake
# ===========================================================================

# ---------------------------------------------------------------------------
# Use a minimum version of CMake of 2.8, >= 2.8.3 is prefered

cmake_minimum_required( VERSION 2.8 )

# ---------------------------------------------------------------------------
# Setup the CMake environment

include( build/CMakeProject.cmake )
include( build/CMakewxAppLib.cmake )

# Specify what wxWidgets libs we need to link to. Note: 'core' must be before 'base'.
# If you call this CMakeLists.txt from another one you may have already called FIND_WXWIDGETS()
if (NOT DEFINED wxWidgets_COMPONENTS)
    set(wxWidgets_COMPONENTS stc html adv core base) # minimal set for static lib/dll
    #set(wxWidgets_COMPONENTS gl xrc xml net media propgrid richtext aui stc html adv core base) # for multilib/dll
    #set(wxWidgets_COMPONENTS stc mono) # for monolithic
endif()

FIND_WXWIDGETS(wxWidgets_COMPONENTS) # Ok to call multiple times
# wxWidgets include (this will do all the magic to configure everything)
include( "${wxWidgets_USE_FILE}" )

# ===========================================================================
# Add subdirectories containing CMakeLists.txt files or specify projects
# ===========================================================================

set( wxStEdit_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Root dir of wxStEdit" FORCE)

# ---------------------------------------------------------------------------
# CMake build file for wxStEdit Library
# ---------------------------------------------------------------------------

if (NOT TARGET wxStEditLib)

set( wxStEdit_VERSION         "1.6.0" )
set( wxStEdit_MAJOR_VERSION   "1" )
set( wxStEdit_MINOR_VERSION   "6" )
set( wxStEdit_RELEASE_VERSION "0" )

# ---------------------------------------------------------------------------

ADD_CPPCHECK_TEST(wxStEdit_CppCheck)

set( DOXYGEN_PROJECT_NAME        "wxStEdit" )
set( DOXYGEN_PROJECT_NUMBER      "${wxStEdit_VERSION}" )
set( DOXYGEN_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/doc-wxStEdit" )
set( DOXYGEN_STRIP_FROM_PATH     "${wxStEdit_ROOT_DIR}/include" )
set( DOXYGEN_STRIP_FROM_INC_PATH "${wxStEdit_ROOT_DIR}/include" )
set( DOXYGEN_INPUT               "${wxStEdit_ROOT_DIR}/include" )
set( DOXYGEN_FILE_PATTERNS       "*.h *.hpp" )
set( DOXYGEN_PREDEFINED          "${DOXYGEN_PREDEFINED_WXWIDGETS} WXDLLIMPEXP_DATA_STEDIT(x)=x")

ADD_DOXYGEN( wxStEdit_doxygen
             ${wxStEdit_ROOT_DIR}/build/CMake-doxygen.in
             ${CMAKE_BINARY_DIR}/doxygen_wxStEdit.cfg
             ${CMAKE_BINARY_DIR} )

# ---------------------------------------------------------------------------

# The user can provide their own "wx/stedit/setup.h" by changing this path
# Also show them where it is so they can modify it.
if (NOT DEFINED wxStEdit_SETUP_INCLUDE_PATH)
    set(wxStEdit_SETUP_INCLUDE_PATH "${CMAKE_BINARY_DIR}/include")
endif()
set(wxStEdit_SETUP_INCLUDE_PATH ${wxStEdit_SETUP_INCLUDE_PATH} CACHE PATH "Path to the configured wxStEdit wx/stedit/setup.h" FORCE)

if (NOT DEFINED wxStEdit_COPY_SETUP0_TO_SETUP_H)
    set(wxStEdit_COPY_SETUP0_TO_SETUP_H TRUE)
endif()
set(wxStEdit_COPY_SETUP0_TO_SETUP_H ${wxStEdit_COPY_SETUP0_TO_SETUP_H} CACHE BOOL "Copy include/wx/stedit/setup0.h file to wxStEdit_SETUP_INCLUDE_PATH/wx/stedit/setup.h overwriting any changes." FORCE)

# Only copy it once so they can modify it and rerun CMake
if (wxStEdit_COPY_SETUP0_TO_SETUP_H)
    configure_file(${wxStEdit_ROOT_DIR}/include/wx/stedit/setup0.h
                   ${wxStEdit_SETUP_INCLUDE_PATH}/wx/stedit/setup.h COPYONLY)
endif()

include_directories(${wxStEdit_ROOT_DIR}/include/ ${wxStEdit_SETUP_INCLUDE_PATH})

set( WXSTEDIT_PROPERTIES_DEFINE_SYMBOL )
if ( BUILD_SHARED_LIBS_WIN_DLLS )
    set( WXSTEDIT_PROPERTIES_DEFINE_SYMBOL PROPERTIES DEFINE_SYMBOL WXMAKINGDLL_STEDIT)
endif()

ADD_LIBRARY_FULL(wxStEditLib
    HEADERS
        ${wxStEdit_SETUP_INCLUDE_PATH}/wx/stedit/setup.h

        ${wxStEdit_ROOT_DIR}/include/wx/stedit/pairarr.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/setup0.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steart.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stedefs.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stedit.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stedlgs.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steevent.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steexprt.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stefindr.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steframe.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stelangs.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stemenum.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stenoteb.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steopts.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steprefs.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steprint.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steshell.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stesplit.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stestyls.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stetree.h
        ${wxStEdit_ROOT_DIR}/src/precomp.h
        ${wxStEdit_ROOT_DIR}/src/wxext.h
        ${wxStEdit_ROOT_DIR}/src/stedlgs_wdr.h
    SOURCES
        ${wxStEdit_ROOT_DIR}/src/precomp.cpp
        ${wxStEdit_ROOT_DIR}/src/steart.cpp
        ${wxStEdit_ROOT_DIR}/src/stedit.cpp
        ${wxStEdit_ROOT_DIR}/src/stedlgs.cpp
        ${wxStEdit_ROOT_DIR}/src/stedlgs_wdr.cpp
        ${wxStEdit_ROOT_DIR}/src/steevent.cpp
        ${wxStEdit_ROOT_DIR}/src/steexprt.cpp
        ${wxStEdit_ROOT_DIR}/src/stefindr.cpp
        ${wxStEdit_ROOT_DIR}/src/steframe.cpp
        ${wxStEdit_ROOT_DIR}/src/stelangs.cpp
        ${wxStEdit_ROOT_DIR}/src/stemenum.cpp
        ${wxStEdit_ROOT_DIR}/src/stenoteb.cpp
        ${wxStEdit_ROOT_DIR}/src/steopts.cpp
        ${wxStEdit_ROOT_DIR}/src/steprefs.cpp
        ${wxStEdit_ROOT_DIR}/src/steprint.cpp
        ${wxStEdit_ROOT_DIR}/src/steshell.cpp
        ${wxStEdit_ROOT_DIR}/src/stesplit.cpp
        ${wxStEdit_ROOT_DIR}/src/stestyls.cpp
        ${wxStEdit_ROOT_DIR}/src/stetree.cpp
        ${wxStEdit_ROOT_DIR}/src/wxext.cpp
    PRECOMPILED_INCLUDE precomp.h
    PRECOMPILED_HEADER  ${wxStEdit_ROOT_DIR}/src/precomp.h
    PRECOMPILED_SOURCE  ${wxStEdit_ROOT_DIR}/src/precomp.cpp
    LINK_LIBRARIES
        ${wxWidgets_LIBRARIES}
    ${WXSTEDIT_PROPERTIES_DEFINE_SYMBOL}
    PROPERTIES FOLDER "wxStEdit")

WXLIKE_LIBRARY_NAMES( wxStEditLib wxstedit "${wxStEdit_VERSION}")

# ---------------------------------------------------------------------------

install(TARGETS wxStEditLib
        EXPORT  wxStEdit_export
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(EXPORT wxStEdit_export
        DESTINATION share/wxstedit/
        FILE wxStEditConfig.cmake)

export( TARGETS wxStEditLib
        FILE "${CMAKE_BINARY_DIR}/build/wxStEditConfig.cmake")

install(FILES
        ${wxStEdit_ROOT_DIR}/docs/ChangeLog.txt
        ${wxStEdit_ROOT_DIR}/docs/readme.txt
        ${wxStEdit_ROOT_DIR}/docs/readme.htm
        ${wxStEdit_ROOT_DIR}/docs/wxstedit.htm
        DESTINATION "doc/wxStEdit")

install(FILES
        ${wxStEdit_SETUP_INCLUDE_PATH}/wx/stedit/setup.h

        ${wxStEdit_ROOT_DIR}/include/wx/stedit/pairarr.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/setup0.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steart.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stedefs.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stedit.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stedlgs.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steevent.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steexprt.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stefindr.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steframe.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stelangs.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stemenum.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stenoteb.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steopts.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steprefs.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steprint.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/steshell.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stesplit.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stestyls.h
        ${wxStEdit_ROOT_DIR}/include/wx/stedit/stetree.h
        DESTINATION "include/wx/stedit")

# ---------------------------------------------------------------------------
# CMake build file for wxStEdit sample
# ---------------------------------------------------------------------------

ADD_EXECUTABLE_FULL( wxStEdit WIN32 MACOSX_BUNDLE
    HEADERS
        ${wxStEdit_ROOT_DIR}/samples/stedit/readme_htm.hpp
        #${wxStEdit_ROOT_DIR}/samples/stedit/stedoc.h
        ${wxStEdit_ROOT_DIR}/samples/stedit/wxstedit.h
        ${wxStEdit_ROOT_DIR}/samples/stedit/wxstedit_htm.hpp
    SOURCES
        #${wxStEdit_ROOT_DIR}/samples/stedit/link.cpp
        #${wxStEdit_ROOT_DIR}/samples/stedit/stedoc.cpp
        ${wxStEdit_ROOT_DIR}/samples/stedit/wxstedit.cpp
        ${wxStEdit_ROOT_DIR}/samples/stedit/wxstedit.rc
    LINK_LIBRARIES
        wxStEditLib
        ${wxWidgets_LIBRARIES}
    PROPERTIES FOLDER "wxStEdit")

if (BUILD_SHARED_LIBS_WIN_DLLS)
    APPEND_TARGET_PROPERTIES(wxStEdit PROPERTIES COMPILE_DEFINITIONS "WXUSINGDLL_STEDIT")
endif()

endif(NOT TARGET wxStEditLib)
