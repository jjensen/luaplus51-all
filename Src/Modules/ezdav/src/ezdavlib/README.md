EZDAV Library
=============

[EZDAV](http://davim.sourceforge.net/ezdav.html) is an offshoot of the [DAVIM](http://davim.sourceforge.net/) project and is written by [Chung Leong](mailto:leong@ocf.berkeley.edu).

EZDAV is a WebDAV client library for Win32. It provides a set of simple functions that makes it easier to write client programs performing rudimentary file operations, such as mkdir, copy and delete. EZDAV is free software, distributed under the Mozilla Public License Version 1.1.

Features
--------

* **Persistent Connection Recovery** - When a persistent HTTP connection is lost or expired, or the server doesn't support persistent connections in the first place, EZDAV automatically reconnects to the server. From the point of view of your application, a persistent connection can last forever. If connection is made through the `dav_connect_lazy` API, the software will try to connect indefinitely.
* **Transparent Lock Handling** - EZDAV handles all the juggling of lock tokens. When a file is itself locked or has inherited a lock from a parent collection, EZDAV automatically supplies the right token.
* **Basic and Digest Authentication**

To-Do List
----------

* Functions for manipulating properties
* Proxy support
* Server-side copy and move
* Progress reporting

Quick Reference
---------------

The relevant functions are defined in `high_level_webdav_functions.h`. They all return boolean results, except for the two which return nothing.

* `int dav_initialize_lock_database(void)` - Initialize the lock database. Should be called prior to any WebDAV operations.
* `void dav_finalize_lock_database(void)` - Stop the lock database.
* `int dav_save_lock_database(const char *filepath)` - Save the lock database to an XML file.
* `int dav_load_lock_database(const char *filepath)` - Load the lock database.
* `int dav_connect(HTTP_CONNECTION **connection, const char *host, short port, const char *username, const char *password)` - Immediately connects to a HTTP server. On failure, `connection` is set to NULL.
* `int dav_connect_lazy(HTTP_CONNECTION **connection, const char *host, short port, const char *username, const char *password)` - Lazily connects to a HTTP server. Reconnection is attempted for all operations and will try indefinitely.
* `int dav_disconnect(HTTP_CONNECTION **connection)` - Disconnect from the host
* `int dav_opendir(HTTP_CONNECTION *connection, const char *directory, DAV_OPENDIR_DATA *oddata)` - Scan a directory for files and sub-directories. Use `dav_readdir` to get the info of each item on the list.
* `int dav_opendir_ex(HTTP_CONNECTION *connection, const char *directory, const char *addition_prop, DAV_OPENDIR_DATA *oddata)` - Like dav_opendir, except you can specify the names of additional properties to scan for. The list of names should be comma-delimited and the names need to include the namespace unless they're DAV properties (e.g. lockdiscovery)
* `int dav_readdir(DAV_OPENDIR_DATA *oddata)` - Get the info of the next item in the directory
* `void dav_closedir(DAV_OPENDIR_DATA *oddata)` - Free up memory allocated by a call to dav_opendir
* `int dav_mkdir(HTTP_CONNECTION *connection, const char *dir)* - Create a new sub-directory in dir
* `int dav_delete(HTTP_CONNECTION *connection, const char *resource)` - Remove from the server a file or a directory
* `int dav_copy_to_server(HTTP_CONNECTION *connection, const char *src, const char *dest)` - Copy a file from the local filesystem to the server
* `int dav_copy_from_server(HTTP_CONNECTION *connection, const char *src, const char *dest)` - Copy a file from the server to the local filesystem
* `int dav_lock(HTTP_CONNECTION *connection, const char *resource, const char *owner)` - Lock a file, so that no other agent can make changes to it. Note that EZDAV only supports exclusive locks
* `int dav_unlock(HTTP_CONNECTION *connection, const char *resource)` - Unlock a file
* `int dav_error(void)` - Error number from the last operation, either an HTTP status code or a system error code (see http.h).
* `const char * dav_error_msg(void)` - A more informative text message.
