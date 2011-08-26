#include <winldap.h>

/* For some reason MSDN mentions LDAP_RES_MODDN, but not LDAP_RES_MODRDN. */
#ifndef LDAP_RES_MODDN
#define LDAP_RES_MODDN LDAP_RES_MODRDN
#endif

/* MSDN doesn't mention LDAP_OPT_SUCCESS, it uses LDAP_SUCCESS intead */
#ifndef LDAP_OPT_SUCCESS
#define LDAP_OPT_SUCCESS LDAP_SUCCESS
#endif

/* MSDN doesn't mention LDAP_SCOPE_DEFAULT, so default will be LDAP_SCOPE_SUBTREE */
#ifndef LDAP_SCOPE_DEFAULT
#define LDAP_SCOPE_DEFAULT LDAP_SCOPE_SUBTREE
#endif

/* MSDN doesn't mention this function at all.  Unfortunately, LDAPMessage an opaque type. */
#define ldap_msgtype(m) ((m)->lm_msgtype)

#define ldap_first_message ldap_first_entry

/* The WinLDAP API allows comparisons against either string or binary values */
#undef ldap_compare_ext

/* The WinLDAP API uses ULONG seconds instead of a struct timeval. */
#undef ldap_search_ext

/* The WinLDAP API has a different number of arguments for this */
#undef ldap_start_tls_s

#ifdef UNICODE
#define ldap_compare_ext(ld,dn,a,v,sc,cc,msg) \
        ldap_compare_extW(ld,dn,a,0,v,sc,cc,msg)
#define ldap_search_ext(ld,base,scope,f,a,o,sc,cc,t,s,msg) \
        ldap_search_extW(ld,base,scope,f,a,o,sc,cc,(t)?(t)->tv_sec:0,s,msg)
#define ldap_start_tls_s(ld,sc,cc) \
        ldap_start_tls_sW(ld,0,0,sc,cc)
#else
#define ldap_compare_ext(ld,dn,a,v,sc,cc,msg) \
        ldap_compare_extA(ld,dn,a,0,v,sc,cc,msg)
#define ldap_search_ext(ld,base,scope,f,a,o,sc,cc,t,s,msg) \
        ldap_search_extA(ld,base,scope,f,a,o,sc,cc,(t)?(t)->tv_sec:0,s,msg)
#define ldap_start_tls_s(ld,sc,cc) \
        ldap_start_tls_sA(ld,0,0,sc,cc)
#endif

