#ifndef __HIGH_LEVEL_WEBDAV_FUNCTIONS__H__
#define __HIGH_LEVEL_WEBDAV_FUNCTIONS__H__
#include "http.h"
#include "webdav.h"

typedef struct dav_opendir_data DAV_OPENDIR_DATA;
#define OD_DIRECTORY	DAV_RESOURCETYPE_COLLECTION
#define OD_FILE			DAV_RESOURCETYPE_OTHER

struct dav_opendir_data {
	const char *href;				// resource info
	DAV_PROP *prop;

	const char *filename;			// info extracted from href and prop
	const char *lockowner;			// for convenience
	int type;
	int size;
	time_t cdate;
	time_t mdate;

	DAV_MULTISTATUS *multistatus;	// internal stuff
	DAV_RESPONSE *response_cursor;
	unsigned int directory_length;
	const char *additional_prop;
};

#ifdef __cplusplus
extern "C" {
#endif

int dav_initialize_lock_database(void);
void dav_finalize_lock_database(void);
int dav_save_lock_database(const char *filepath);
int dav_load_lock_database(const char *filepath);

int dav_connect(HTTP_CONNECTION **connection, const char *host, short port, const char *username, const char *password);
int dav_connect_lazy(HTTP_CONNECTION **connection, const char *host, short port, const char *username, const char *password);
int dav_disconnect(HTTP_CONNECTION **connection);

int dav_opendir(HTTP_CONNECTION *connection, const char *directory, DAV_OPENDIR_DATA *oddata);
int dav_opendir_ex(HTTP_CONNECTION *connection, const char *directory, const char *addition_prop, DAV_OPENDIR_DATA *oddata);
int dav_readdir(DAV_OPENDIR_DATA *oddata);
void dav_closedir(DAV_OPENDIR_DATA *oddata);
int dav_attributes(HTTP_CONNECTION *connection, const char *path, DAV_OPENDIR_DATA *oddata);
int dav_attributes_ex(HTTP_CONNECTION *connection, const char *path, const char *addition_prop, DAV_OPENDIR_DATA *oddata);

int dav_mkdir(HTTP_CONNECTION *connection, const char *dir);
int dav_delete(HTTP_CONNECTION *connection, const char *resource);
int dav_copy_to_server(HTTP_CONNECTION *connection, const char *src, const char *dest);
int dav_copy_from_server(HTTP_CONNECTION *connection, const char *src, const char *dest);
int dav_lock(HTTP_CONNECTION *connection, const char *resource, const char *owner);
int dav_unlock(HTTP_CONNECTION *connection, const char *resource);
int dav_abandon_lock(HTTP_CONNECTION *connection, const char *resource);

int dav_error(HTTP_CONNECTION *connection);
const char * dav_error_msg(HTTP_CONNECTION *connection);

#ifdef __cplusplus
}
#endif

#endif
