#ifndef __WEBDAV_H__
#define __WEBDAV_H__
#include <time.h>
#include "xml_tree.h"

typedef struct dav_propfind DAV_PROPFIND;
typedef int DAV_ALLPROP;
typedef int DAV_PROPNAME;
typedef struct dav_multistatus DAV_MULTISTATUS;
typedef struct dav_response DAV_RESPONSE;
typedef struct dav_propstat DAV_PROPSTAT;
typedef struct dav_prop DAV_PROP;
typedef int DAV_RESOURCETYPE;
typedef struct dav_supportedlock DAV_SUPPORTEDLOCK;
typedef struct dav_lockentry DAV_LOCKENTRY;
typedef int DAV_LOCKSCOPE;
typedef int DAV_LOCKTYPE;
typedef struct dav_lockinfo DAV_LOCKINFO;
typedef struct dav_lockdiscovery DAV_LOCKDISCOVERY;
typedef struct dav_activelock DAV_ACTIVELOCK;

#define DAV_RESOURCETYPE_COLLECTION	1
#define DAV_RESOURCETYPE_OTHER		0

#define DAV_LOCKSCOPE_SHARED		1
#define DAV_LOCKSCOPE_EXCLUSIVE		2
#define DAV_LOCKSCOPE_UNKNOWN		0

#define DAV_LOCKTYPE_WRITE			1
#define DAV_LOCKTYPE_UNKNOWN		0

struct dav_propfind {
	DAV_PROP *prop;
	DAV_ALLPROP allprop;
	DAV_PROPNAME propname;
};

struct dav_multistatus {
	DAV_RESPONSE *first_response;
	DAV_RESPONSE *last_response;
};

struct dav_response {
	char *href;
	DAV_PROPSTAT *first_propstat;
	DAV_PROPSTAT *last_propstat;
	DAV_RESPONSE *prev_response;
	DAV_RESPONSE *next_response;
};

struct dav_propstat {
	int status_code;
	char *status_msg;
	DAV_PROP *prop;
	DAV_PROPSTAT *prev_propstat;
	DAV_PROPSTAT *next_propstat;
};

struct dav_prop {
	time_t creationdate;
	time_t getlastmodified;
	char *displayname;
	DAV_RESOURCETYPE resourcetype;
	DAV_SUPPORTEDLOCK *supportedlock;
	DAV_LOCKDISCOVERY *lockdiscovery;
	int getcontentlength;
	char *getcontenttype;
	int status_code;
	char *status_msg;
	XML_NODE *first_external_prop;
	XML_NODE *last_external_prop;
};

struct dav_supportedlock {
	DAV_LOCKENTRY *first_lockentry;
	DAV_LOCKENTRY *last_lockentry;
};

struct dav_lockentry {
	DAV_LOCKSCOPE lockscope;
	DAV_LOCKTYPE locktype;
	DAV_LOCKENTRY *prev_lockentry;
	DAV_LOCKENTRY *next_lockentry;
};

struct dav_lockinfo {
	DAV_LOCKSCOPE lockscope;
	DAV_LOCKTYPE locktype;
	char *owner;
};

struct dav_lockdiscovery {
	DAV_ACTIVELOCK *first_activelock;
	DAV_ACTIVELOCK *last_activelock;
};

struct dav_activelock {
	DAV_LOCKSCOPE lockscope;
	DAV_LOCKTYPE locktype;
	char *owner;
	int depth;
	char *timeout;
	char *locktoken;
	DAV_ACTIVELOCK *prev_activelock;
	DAV_ACTIVELOCK *next_activelock;
};

int dav_create_propfind(DAV_PROPFIND **propfind);
int dav_add_find_custom_prop(DAV_PROPFIND *propfind, const char *name, const char *ns);
int dav_add_find_prop(DAV_PROPFIND *propfind, const char *name);
int dav_add_find_prop_comma_delimited(DAV_PROPFIND *propfind, const char *additional_prop);
int dav_set_find_all_prop(DAV_PROPFIND *propfind);
int dav_write_propfind_to_storage(DAV_PROPFIND *propfind, HTTP_STORAGE *storage);
void dav_propfind_destroy(DAV_PROPFIND **propfind);

int dav_create_multistatus_from_storage(DAV_MULTISTATUS **multistatus, HTTP_STORAGE *storage);
void dav_multistatus_destroy(DAV_MULTISTATUS **multistatus);
DAV_PROP *dav_find_prop(DAV_RESPONSE *response, int status_code_range_start, int status_code_range_end);
DAV_ACTIVELOCK *dav_find_activelock(DAV_PROP *prop, DAV_LOCKSCOPE lockscope, DAV_LOCKTYPE locktype);

int dav_create_lockinfo(DAV_LOCKINFO **lockinfo, DAV_LOCKSCOPE lockscope, DAV_LOCKTYPE locktype, const char *owner);
int dav_write_lockinfo_to_storage(DAV_LOCKINFO *lockinfo, HTTP_STORAGE *storage);
int dav_create_prop_from_storage(DAV_PROP **prop, HTTP_STORAGE *storage);
void dav_lockinfo_destroy(DAV_LOCKINFO **lockinfo);
void dav_prop_destroy(DAV_PROP **prop);

int http_propfind(HTTP_CONNECTION *connection, const char *resource, DAV_PROPFIND *propfind, int depth, DAV_MULTISTATUS **multistatus);
int http_mkcol(HTTP_CONNECTION *connection, const char *resource);
int http_delete(HTTP_CONNECTION *connection, const char *resource, DAV_MULTISTATUS **multistatus);
int http_lock(HTTP_CONNECTION *connection, const char *resource, DAV_LOCKINFO *lockinfo, int depth, DAV_MULTISTATUS **multistatus);

#endif
