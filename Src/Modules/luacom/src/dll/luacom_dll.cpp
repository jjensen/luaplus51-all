
#include <windows.h>
extern "C" {
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
#ifdef IUP
  #include "iup.h"
  #ifndef NO_CPICONTROLS
  #include "iupcontrols.h"
  #include "iupgl.h"
  #include "iupluaim.h"
  #endif
  #ifdef SPEECH_SDK
  #include "iupspeech.h"
  #endif
  #include "iuplua.h"
  #include "cdlua5.h"
  #ifndef NO_CPICONTROLS
  #include "luacontrols.h"
  #include "luagl.h"
  #endif
  #ifdef SPEECH_SDK
  #include "luaspeech.h"
  #endif
  #ifdef USE_GLLUA
  #include "gllua.h"
  #endif
#endif
}
#include "LuaCompat.h"
#include "luacom.h"
#include "luacom_internal.h"
#include "tLuaCOMClassFactory.h"
#include "tLuaCOMException.h"
#include "tUtil.h"
#include "tCOMUtil.h"

extern "C" int luacom_openlib(lua_State* L) {
  luacom_open(L);
  return 0;
}

extern "C" int luaopen_luacom(lua_State* L) {
  return luacom_openlib(L);
}


#define lua_dofile luaL_dofile


static lua_State* factoryCache; // Maps CLSIDs to factories
CRITICAL_SECTION cs_factoryCache;

static void factoryCache_Init() {
  InitializeCriticalSection(&cs_factoryCache);

#ifdef IUP
  /* iup initialization */
  IupOpen();
  #ifndef NO_CPICONTROLS
  IupGLCanvasOpen();
  IupControlsOpen();
  #endif
  #ifdef SPEECH_SDK
  IupSpeechOpen();
  #endif
#endif
  factoryCache = luaL_newstate();
  lua_newtable(factoryCache);
  lua_setglobal(factoryCache,"factories");
}

static tLuaCOMClassFactory* factoryCache_GetFactory(const char* luaclsid) {
  CriticalSectionObject cs(&cs_factoryCache); // won't let other threads enter till it goes out of scope
  lua_getglobal(factoryCache,"factories");
  lua_pushstring(factoryCache,luaclsid);
  lua_gettable(factoryCache,-2);
  tLuaCOMClassFactory* pFactory = (tLuaCOMClassFactory*)luaCompat_getPointer(factoryCache,-1);
  lua_pop(factoryCache,2);
  return pFactory;
}

static void factoryCache_PutFactory(const char* luaclsid, tLuaCOMClassFactory* pFactory) {
  CriticalSectionObject cs(&cs_factoryCache); // won't let other threads enter till it goes out of scope
  pFactory->AddRef();
  lua_getglobal(factoryCache,"factories");
  lua_pushstring(factoryCache,luaclsid);
  lua_pushlightuserdata(factoryCache,pFactory);
  lua_settable(factoryCache,-3);
  lua_pop(factoryCache,1);
}

static void factoryCache_Release() {
  CriticalSectionObject cs(&cs_factoryCache); // won't let other threads enter till it goes out of scope
  lua_getglobal(factoryCache,"factories");
    lua_pushnil(factoryCache); 
    while(lua_next(factoryCache,-2) != 0) {
    tLuaCOMClassFactory* pFactory = (tLuaCOMClassFactory*)luaCompat_getPointer(factoryCache,-1);
    pFactory->Release();
    lua_pop(factoryCache,1);
    }
  lua_pop(factoryCache,1);
  #ifdef IUP
    IupClose();
  #endif
}

static lua_State* luacom_DoRegistryFile(const char* luaclsid) {
  lua_State* L_inproc = NULL;
  char* fileName;
  char* key = new char[strlen(luaclsid)+18];

  strcpy(key, "CLSID\\");
  strcat(key,luaclsid);
  strcat(key,"\\ScriptFile");

  if(tCOMUtil::GetRegKeyValue(key,&fileName)) {
    L_inproc = luaL_newstate();
    luaL_openlibs(L_inproc);
    #ifdef IUP
      /* iuplua initialization */
      iuplua_open(L_inproc);
      iupkey_open(L_inproc);
      cdlua5_open(L_inproc);
      #ifndef NO_CPICONTROLS
      controlslua_open(L_inproc);
      gllua_open(L_inproc);
      #endif
      #ifdef SPEECH_SDK
      speechlua_open(L_inproc);
      #endif
      #ifdef USE_GLLUA
      gl_open(L_inproc);
      #endif
      iupluaim_open(L_inproc);
    #endif

    luacom_open(L_inproc);
    lua_pushvalue(L_inproc, LUA_REGISTRYINDEX);
    lua_pushstring(L_inproc,"inproc");
    lua_pushboolean(L_inproc,TRUE);
    lua_settable(L_inproc,-3);
    lua_pop(L_inproc,1);

    if(!lua_dofile(L_inproc,fileName)) {
      lua_pushstring(L_inproc,"StartAutomation");
      lua_gettable(L_inproc,-2);
      if(luaCompat_call(L_inproc, 0, 0)) {
        luacom_close(L_inproc);
        lua_close(L_inproc);
        L_inproc = NULL;
      }
    } else {
      luacom_close(L_inproc);
      lua_close(L_inproc);
      L_inproc = NULL;
    }
  }

  delete[] key;
  SAFEDELETEARR(fileName);
  return L_inproc;
}

static tLuaCOMClassFactory* luacom_GetInprocFactory(REFCLSID rclsid) {
  BSTR clsid;
  try {
    CHK_COM_CODE(StringFromCLSID(rclsid, &clsid));
    tStringBuffer luaclsid = tUtil::bstr2string(clsid);
    CoTaskMemFree(clsid);
    tLuaCOMClassFactory* pFactory = factoryCache_GetFactory(luaclsid);
    if(pFactory == NULL) {
      lua_State* L_inproc = luacom_DoRegistryFile(luaclsid);
      if(L_inproc != NULL) {
        pFactory = new tLuaCOMClassFactory(L_inproc);
        factoryCache_PutFactory(luaclsid,pFactory);
      }
    }
    return pFactory;
   } catch(class tLuaCOMException&) {
      return NULL;
  }
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
  tLuaCOMClassFactory* pFactory = luacom_GetInprocFactory(rclsid);
  if(pFactory == NULL) return CLASS_E_CLASSNOTAVAILABLE;

  HRESULT hr = pFactory->QueryInterface(riid,ppv);

  if(FAILED(hr)) return hr;

  return S_OK;
}

extern "C"
BOOL WINAPI DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
           )
{
  g_hInstance = (HINSTANCE)hModule;
  if(ul_reason_for_call == DLL_PROCESS_ATTACH) {
    OleInitialize(NULL);
    factoryCache_Init();
  } else if(ul_reason_for_call == DLL_PROCESS_DETACH) {
    factoryCache_Release();
    OleUninitialize();
  }
    return TRUE;
}
