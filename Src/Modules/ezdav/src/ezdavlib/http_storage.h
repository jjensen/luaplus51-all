#ifndef __HTTP_STORAGE_H__
#define __HTTP_STORAGE_H__
#include <stdio.h>

typedef struct http_storage HTTP_STORAGE;
typedef struct http_memory_storage HTTP_MEMORY_STORAGE;
typedef struct http_file_storage HTTP_FILE_STORAGE;
typedef struct http_offset_file_storage HTTP_OFFSET_FILE_STORAGE;

typedef int (*HTTP_STORAGE_WRITE)(HTTP_STORAGE *storage, const char *data, int length);
typedef int (*HTTP_STORAGE_READ)(HTTP_STORAGE *storage, char *buffer, int buffer_size, int *count);
typedef int (*HTTP_STORAGE_SEEK)(HTTP_STORAGE *storage, int location);
typedef int (*HTTP_STORAGE_GETSIZE)(HTTP_STORAGE *storage, int *size);
typedef int (*HTTP_STORAGE_CLOSE)(HTTP_STORAGE *storage);
typedef void (*HTTP_STORAGE_DESTROY)(HTTP_STORAGE *storage);

struct http_storage {
	HTTP_STORAGE_WRITE write;
	HTTP_STORAGE_READ read;
	HTTP_STORAGE_SEEK seek;
	HTTP_STORAGE_GETSIZE getsize;
	HTTP_STORAGE_CLOSE close;
	HTTP_STORAGE_DESTROY destroy;
};

struct http_memory_storage {
	HTTP_STORAGE functions;
	char *content;
	int content_index;
	int content_size;
	int content_buffer_size;
	int max_content_buffer_size;
};

struct http_file_storage {
	HTTP_STORAGE functions;
	FILE *file;
	int file_size;
};

struct http_offset_file_storage {
	HTTP_STORAGE functions;
	FILE *file;
	int file_size;
	size_t* chunk_offsets;
	size_t* chunk_sizes;
	int number_of_chunks;
	int current_chunk_index;
	int current_chunk_offset;
};

int http_create_memory_storage(HTTP_MEMORY_STORAGE **storage, char* content, int max_content_buffer_size);
int http_create_file_storage(HTTP_FILE_STORAGE **storage, const char *filename, const char *mode);
void http_destroy_generic_storage(HTTP_STORAGE **storage);

#define http_storage_write(s, d, l)	((HTTP_STORAGE *) s)->write((HTTP_STORAGE *) s, d, l)
#define http_storage_read(s, b, l, c) ((HTTP_STORAGE *) s)->read((HTTP_STORAGE *) s, b, l, c)
#define http_storage_getsize(s, sz) ((HTTP_STORAGE *) s)->getsize((HTTP_STORAGE *) s, sz)
#define http_storage_seek(s, l) ((HTTP_STORAGE *) s)->seek((HTTP_STORAGE *) s, l)
#define http_storage_close(s) ((HTTP_STORAGE *) s)->close((HTTP_STORAGE *) s)
#define http_storage_destroy(sp) http_destroy_generic_storage((HTTP_STORAGE **) sp);

#endif
