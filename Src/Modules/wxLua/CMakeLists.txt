INCLUDE(../../../LuaPlusBuildTypes.cmake)

FIND_PACKAGE(wxWidgets COMPONENTS xrc html adv net xml core base aui media) # stc)
INCLUDE( ${wxWidgets_USE_FILE} )

SET(SRCS
	modules/luamodule/src/luamodule.cpp
	modules/luamodule/include/luamoduledefs.h
	modules/wxbind/include/wxbinddefs.h
	
	# wxbindadv
	modules/wxbind/include/wxadv_bind.h
	modules/wxbind/src/wxadv_bind.cpp
	modules/wxbind/src/wxadv_wxladv.cpp

	# wxbindaui
	modules/wxbind/include/wxaui_bind.h
	modules/wxbind/src/wxaui_bind.cpp

	# wxbindbase
	modules/wxbind/include/wxbase_bind.h
	modules/wxbind/src/wxbase_base.cpp
	modules/wxbind/src/wxbase_bind.cpp
	modules/wxbind/src/wxbase_config.cpp
	modules/wxbind/src/wxbase_data.cpp
	modules/wxbind/src/wxbase_datetime.cpp
	modules/wxbind/src/wxbase_file.cpp

	# wxbindcore
	modules/wxbind/include/wxcore_bind.h
	modules/wxbind/src/wxcore_appframe.cpp
	modules/wxbind/src/wxcore_bind.cpp
	modules/wxbind/src/wxcore_clipdrag.cpp
	modules/wxbind/src/wxcore_controls.cpp
	modules/wxbind/src/wxcore_core.cpp
	modules/wxbind/src/wxcore_defsutils.cpp
	modules/wxbind/src/wxcore_dialogs.cpp
	modules/wxbind/src/wxcore_event.cpp
	modules/wxbind/src/wxcore_gdi.cpp
	modules/wxbind/src/wxcore_geometry.cpp
	modules/wxbind/src/wxcore_help.cpp
	modules/wxbind/src/wxcore_image.cpp
	modules/wxbind/src/wxcore_mdi.cpp
	modules/wxbind/src/wxcore_menutool.cpp
	modules/wxbind/src/wxcore_picker.cpp
	modules/wxbind/src/wxcore_print.cpp
	modules/wxbind/src/wxcore_sizer.cpp
	modules/wxbind/src/wxcore_windows.cpp
	modules/wxbind/src/wxcore_wxlcore.cpp

	# wxbindgl
	modules/wxbind/include/wxgl_bind.h
	modules/wxbind/src/wxgl_bind.cpp

	# wxbindhtml
	modules/wxbind/include/wxhtml_bind.h
	modules/wxbind/include/wxhtml_wxlhtml.h
	modules/wxbind/src/wxhtml_bind.cpp
	modules/wxbind/src/wxhtml_wxlhtml.cpp

	# wxbindmedia
	modules/wxbind/include/wxmedia_bind.h
	modules/wxbind/src/wxmedia_bind.cpp

	# wxbindnet
	modules/wxbind/include/wxnet_bind.h
	modules/wxbind/src/wxnet_bind.cpp

	# wxbindrichtext
	modules/wxbind/include/wxrichtext_bind.h
	modules/wxbind/src/wxrichtext_bind.cpp

	# wxbindstc
	modules/wxbind/include/wxstc_bind.h
	modules/wxbind/src/wxstc_bind.cpp

	# wxbindxml
	modules/wxbind/include/wxxml_bind.h
	modules/wxbind/src/wxxml_bind.cpp

	# wxbindxrc
	modules/wxbind/include/wxxrc_bind.h
	modules/wxbind/src/wxxrc_bind.cpp

	# wxlua
	modules/wxlua/include/wxlbind.h
	modules/wxlua/include/wxlcallb.h
	modules/wxlua/include/wxldefs.h
	modules/wxlua/include/wxlstate.h
	modules/wxlua/include/wxlua.h
	modules/wxlua/include/wxlua_bind.h
	modules/wxlua/src/wxlbind.cpp
	modules/wxlua/src/wxlcallb.cpp
	modules/wxlua/src/wxlstate.cpp
	modules/wxlua/src/wxlua_bind.cpp

	# wxluadebug
	modules/wxluadebug/include/wxldebug.h
	modules/wxluadebug/include/wxlstack.h
	modules/wxluadebug/include/wxluadebugdefs.h
	modules/wxluadebug/src/wxldebug.cpp
	modules/wxluadebug/src/wxlstack.cpp

	# wxluasocket
	modules/wxluasocket/include/wxldserv.h
	modules/wxluasocket/include/wxldtarg.h
	modules/wxluasocket/include/wxlsock.h
	modules/wxluasocket/include/wxluasocket_bind.h
	modules/wxluasocket/include/wxluasocketdefs.h
	modules/wxluasocket/src/dummy.cpp
	modules/wxluasocket/src/wxldserv.cpp
	modules/wxluasocket/src/wxldtarg.cpp
	modules/wxluasocket/src/wxlsock.cpp
#	modules/wxluasocket/src/wxluasocket.cpp
	modules/wxluasocket/src/wxluasocket_bind.cpp
)

ADD_DEFINITIONS(-DWXMAKINGDLL_LUAMODULE)

INCLUDE_DIRECTORIES(. modules/ modules/wxbind/setup/)
INCLUDE_DIRECTORIES(${wxWidgets_ROOT_DIR}/contrib/include)

LUAPLUS_ADD_MODULE(wx ${SRCS})
TARGET_LINK_LIBRARIES(wx comctl32)
TARGET_LINK_LIBRARIES(wx ${wxWidgets_LIBRARIES})
TARGET_LINK_LIBRARIES(wx debug ${wxWidgets_LIB_DIR}/wxmsw28d_stc.lib)
TARGET_LINK_LIBRARIES(wx optimized ${wxWidgets_LIB_DIR}/wxmsw28_stc.lib)
