/******************************************************************************
* Copyright (C) 2007 Juergen Hoetzel
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#include "Lua-cURL.h"
#include "Lua-utility.h"

/* malloc */
#include <stdlib.h>

#if LUA_VERSION_NUM < 502
  #define luaL_newlibtable(L,l) lua_createtable(L,0,sizeof(l)/sizeof((l)[0]))
  #define luaL_newlib(L,l) (luaL_newlibtable(L,l),luaL_register(L,NULL,l))
  #define luaL_setfuncs(L,l,nups) luaL_register(L,NULL,l)
  #define lua_pushunsigned(L,val) lua_pushinteger(L,val)
#endif

/* methods assigned to easy table */
static const struct luaL_Reg luacurl_easy_m[] = {
  {"escape", l_easy_escape},
  {"perform", l_easy_perform},
  {"unescape", l_easy_unescape},
  {"post", l_easy_post},
  {"__gc", l_easy_gc},
  /* not for public use */
  {NULL, NULL}};

/* methods assigned to multi table */
static const struct luaL_Reg luacurl_multi_m[] = {
  {"add_handle", l_multi_add_handle},
  {"perform", l_multi_perform},
  {"__gc", l_multi_gc},
  {NULL, NULL}};

static const struct luaL_Reg luacurl_share_m[] = {
  {"setopt_share", l_share_setopt_share},
  {"__gc", l_share_gc},
  {NULL, NULL}};

/* global functions in module namespace*/
static const struct luaL_Reg luacurl_f[] = {
  {"easy_init", l_easy_init},
  {"multi_init", l_multi_init},
  {"share_init", l_share_init},
  {"getdate", l_getdate},
  {"unescape", l_unescape},
  {"version", l_version},
  {"version_info", l_version_info},
  {NULL, NULL}};

/* functions assigned to metatable */
static const struct luaL_Reg luacurl_m[] = {

  {NULL, NULL}};

/* (key,value) items in the cURL.protocols table */
static const struct {
    char *name;             /* human readable name: used as table index */
    unsigned long value;    /* CURLPROTO_* value */
} luacurl_protos[] = {
    {"HTTP", CURLPROTO_HTTP},
    {"HTTPS", CURLPROTO_HTTPS},
    {"FTP", CURLPROTO_FTP},
    {"FTPS", CURLPROTO_FTPS},
    {"SCP", CURLPROTO_SCP},
    {"SFTP", CURLPROTO_SFTP},
    {"TELNET", CURLPROTO_TELNET},
    {"LDAP", CURLPROTO_LDAP},
    {"LDAPS", CURLPROTO_LDAPS},
    {"DICT", CURLPROTO_DICT},
    {"FILE", CURLPROTO_FILE},
    {"TFTP", CURLPROTO_TFTP},
    {"IMAP", CURLPROTO_IMAP},
    {"IMAPS", CURLPROTO_IMAPS},
    {"POP3", CURLPROTO_POP3},
    {"POP3S", CURLPROTO_POP3S},
    {"SMTP", CURLPROTO_SMTP},
    {"SMTPS", CURLPROTO_SMTPS},
    {"RTSP", CURLPROTO_RTSP},
    {"RTMP", CURLPROTO_RTMP},
    {"RTMPT", CURLPROTO_RTMPT},
    {"RTMPE", CURLPROTO_RTMPE},
    {"RTMPTE", CURLPROTO_RTMPTE},
    {"RTMPS", CURLPROTO_RTMPS},
    {"RTMPTS", CURLPROTO_RTMPTS},
    {"GOPHER", CURLPROTO_GOPHER},
    {"ALL", CURLPROTO_ALL},
    {NULL, 0}
};

int l_easy_escape(lua_State *L) {
  size_t length = 0;
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  const char *url = luaL_checklstring(L, 2, &length);
  char *rurl = curl_easy_escape(curl, url, length);
  lua_pushstring(L, rurl);
  curl_free(rurl);
  return 1;
}


int l_easy_perform(lua_State *L) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  /* do writecallback? */
  int writefunction;
  /* do headercallback */
  int headerfunction;
  /* do readcallback */
  int readfunction;
  /* curl_easy_perform return code*/
  CURLcode perform_status;

  /* check optional callback table */
  luaL_opt(L, luaL_checktable, 2, lua_newtable(L));

  /* setup write callback function only if entry exists in callback-table */
  lua_getfield(L, 2, "writefunction");
  writefunction =  lua_isfunction(L, -1)?1:0;
  if (writefunction)
    l_easy_setup_writefunction(L, privatep->curl);
  lua_pop(L, 1);

  /* setup header callback function only if entry exists in callback-table */
  lua_getfield(L, 2, "headerfunction");
  headerfunction = lua_isfunction(L, -1)?1:0;
  if (headerfunction)
    l_easy_setup_headerfunction(L, privatep->curl);
  lua_pop(L, 1);

  /* setup read callback function only if entry exists in callback-table */
  lua_getfield(L, 2, "readfunction");
  readfunction = lua_isfunction(L, -1)?1:0;
  if (readfunction)
    l_easy_setup_readfunction(L, privatep->curl);
  lua_pop(L, 1);


  /* callback table is on top on stack */
  perform_status = curl_easy_perform(curl);

  /* unset callback functions */
  if (headerfunction)
    l_easy_clear_headerfunction(L, privatep->curl);
  if (writefunction)
    l_easy_clear_writefunction(L, privatep->curl);
  if (readfunction)
    l_easy_clear_readfunction(L, privatep->curl);

  if (perform_status != CURLE_OK)
    return luaL_error(L, "%s", privatep->error);

  return 0;
}

int l_easy_init(lua_State *L) {
  l_easy_private *privp;

  /* create userdata and assign metatable */
  privp = (l_easy_private *) lua_newuserdata(L, sizeof(l_easy_private));

  /* allocate list of curl_slist for setopt handling */
  l_easy_setopt_init_slists(L, privp);

  luaL_getmetatable(L, LUACURL_EASYMETATABLE);
  lua_setmetatable(L, - 2);

  if ((privp->curl = curl_easy_init()) == NULL)
    return luaL_error(L, "something went wrong and you cannot use the other curl functions");

  /* set error buffer */
  if (curl_easy_setopt(privp->curl, CURLOPT_ERRORBUFFER, privp->error) != CURLE_OK)
    return luaL_error(L, "cannot set error buffer");

  /* return userdata; */
  return 1;
}

int l_getdate(lua_State *L) {
  const char *date = luaL_checkstring(L, 1);
  time_t t = curl_getdate(date, NULL);
  if (t == -1)
    return luaL_error(L, "fails to parse the date string");
  lua_pushinteger(L, t);
  return 1;
}


int l_easy_unescape(lua_State *L) {
  size_t inlength = 0;
  int outlength;
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  const char *url = luaL_checklstring(L, 2, &inlength);
  char *rurl = curl_easy_unescape(curl, url, inlength, &outlength);
  lua_pushlstring(L, rurl, outlength);
  curl_free(rurl);
  return 1;
}

/* deprecated */
int l_unescape(lua_State *L) {
  size_t length;
  const char *url = luaL_checklstring(L, 1, &length);
  char *rurl = curl_unescape(url, length);
  lua_pushstring(L, rurl);
  curl_free(rurl);
  return 1;
}


int l_version(lua_State *L) {
  lua_pushstring(L, curl_version());
  return 1;
}

int l_version_info (lua_State *L) {
  int i;
  curl_version_info_data *d = curl_version_info(CURLVERSION_NOW);
  struct {char *name; int bitmask;} features[] = {
    {"ipv6", CURL_VERSION_IPV6},
    {"kerberos4", CURL_VERSION_KERBEROS4},
    {"ssl", CURL_VERSION_SSL},
    {"libz", CURL_VERSION_LIBZ},
    {"ntlm",CURL_VERSION_NTLM},
    {"gssnegotiate",CURL_VERSION_GSSNEGOTIATE},
    {"debug",CURL_VERSION_DEBUG},
    {"asynchdns",CURL_VERSION_ASYNCHDNS},
    {"spnego",CURL_VERSION_SPNEGO},
    {"largefile",CURL_VERSION_LARGEFILE},
    {"idn",CURL_VERSION_IDN},
    {"sspi",CURL_VERSION_SSPI},
    {"conv",CURL_VERSION_CONV},
    {NULL, 0}
  };



  lua_newtable(L);

  lua_pushliteral(L, "version");
  lua_pushstring(L, d->version);
  lua_settable(L, -3);

  lua_pushliteral(L, "version_num");
  lua_pushinteger(L, d->version_num);
  lua_settable(L, -3);

  lua_pushliteral(L, "host");
  lua_pushstring(L, d->host);
  lua_settable(L, -3);

  /* create features table */
  lua_pushliteral(L, "features");
  lua_newtable(L);

  i = 0;
  while (features[i].name != NULL) {
    lua_pushboolean(L, d->features & features[i].bitmask);
    lua_setfield(L, -2, features[i++].name);
  }

  lua_settable(L, -3);

  /* ssl */
  lua_pushliteral(L, "ssl_version");
  lua_pushstring(L, d->ssl_version);
  lua_settable(L, -3);

  lua_pushliteral(L, "libz_version");
  lua_pushstring(L, d->libz_version);
  lua_settable(L, -3);

  /* create protocols table*/
  lua_pushstring(L,"protocols");
  lua_newtable(L);

  for(i=0; d->protocols[i] != NULL; i++){
	lua_pushinteger(L, i+1);
	lua_pushstring(L, d->protocols[i]);
	lua_settable(L, -3);
  }

  lua_settable(L, -3);

  if (d->age >= 1) {
    lua_pushliteral(L, "ares");
    lua_pushstring(L, d->ares);
    lua_settable(L, -3);

    lua_pushliteral(L, "ares_num");
    lua_pushinteger(L, d->ares_num);
    lua_settable(L, -3);
  }

  if (d->age >= 2) {
    lua_pushliteral(L, "libidn");
    lua_pushstring(L, d->libidn);
    lua_settable(L, -3);
  }

  if (d->age >= 3) {
    lua_pushliteral(L, "iconv_ver_num");
    
    lua_pushinteger(L, d->iconv_ver_num);
    lua_settable(L, -3);
  }

  /* return table*/
  return 1;
}

int l_easy_gc(lua_State *L) {
  /* gc resources optained by cURL userdata */
  l_easy_private *privp = lua_touserdata(L, 1);
  curl_easy_cleanup(privp->curl);
  l_easy_setopt_free_slists(privp);
  return 0;
}

/*
 * Registers a table named "protocols" in the table on top of the stack.
 *
 * This function is meant for local use only. It's main use is for keeping the
 * module registration function clean
 */
static void l_protocols_register(lua_State *L) {
    size_t i;

    lua_createtable(L, 0, sizeof luacurl_protos / sizeof luacurl_protos[0]);

    for(i = 0; luacurl_protos[i].name != NULL; i++) {
        lua_pushunsigned(L, luacurl_protos[i].value);
        lua_setfield(L, -2, luacurl_protos[i].name);
    }

    lua_setfield(L, -2, "protocols");
}

/* registration hook function */
LUACURL_API int luaopen_cURL(lua_State* L) {
  CURLcode  rc;

  /* EASY START */
  luaL_newmetatable(L, LUACURL_EASYMETATABLE);

  /* register in easymetatable */
  luaL_setfuncs(L, luacurl_easy_m, 0);


  /* easymetatable.__index = easymetatable */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  /* register getinfo closures  */
  l_easy_getinfo_register(L);
  /* register setopt closures  */
  l_easy_setopt_register(L);


  /* SHARE START */
  luaL_newmetatable(L, LUACURL_SHAREMETATABLE);

  /* register in sharemetatable */
  luaL_setfuncs(L, luacurl_share_m, 0);

  /* sharemetatable.__index = sharemetatable */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  /* MULTI START */
  luaL_newmetatable(L, LUACURL_MULTIMETATABLE);
  luaL_setfuncs(L, luacurl_multi_m, 0);
    /* multemetatable.__index = multimetatable */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");


  /* creaty uniqe table in registry to store state*/
  lua_newtable(L);
  /*weak table*/
  /*
  lua_pushvalue(L,-1);
  lua_pushliteral(L,"v");
  lua_setfield(L,-2,"__mode");
  lua_setmetatable(L,-2);
  */
  lua_setfield(L, LUA_REGISTRYINDEX, MULTIREGISTRY_KEY);
  lua_pop(L, 1);		/* pop table */


  /* return module functions */
  luaL_newlib(L, luacurl_f);
  lua_pushvalue(L,-1);
  lua_setglobal(L,LUACURL_LIBNAME);

  /* set protocols table */
  l_protocols_register(L);

  /* initialize curl once */
  if ((rc = curl_global_init(CURL_GLOBAL_ALL)) != CURLE_OK)
    luaL_error(L, "curl_global_init: %s", curl_easy_strerror(rc));
  return 1;
}
