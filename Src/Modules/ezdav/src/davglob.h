/**
	\file davglob.h
**/
#ifndef __DAVGLOB_H__
#define __DAVGLOB_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
	typedef unsigned __int64 davglob_uint64;
#else
	typedef unsigned long long davglob_uint64;
#endif

#if !defined(FILEGLOB_BUILD_IMPLEMENTATION)
typedef struct _davglob davglob;
typedef void* (*davglob_Alloc)(void* userData, void* ptr, unsigned int size);
#endif

davglob* davglob_Create(HTTP_CONNECTION* connection, const char* inPattern);
davglob* davglob_CreateWithAlloc(HTTP_CONNECTION* connection, const char* inPattern, davglob_Alloc alloc, void* userData);
void davglob_Destroy(davglob* self);
void davglob_AddExclusivePattern(davglob* self, const char* name);
void davglob_AddIgnorePattern(davglob* self, const char* name);

int davglob_Next(davglob* self);

const char* davglob_FileName(davglob* self);
davglob_uint64 davglob_CreationTime(davglob* self);
davglob_uint64 davglob_WriteTime(davglob* self);
davglob_uint64 davglob_FileSize(davglob* self);
int davglob_IsDirectory(davglob* self);

int davglob_WildMatch(const char* pattern, const char *string, int caseSensitive);

#if defined(_WIN32)  &&  defined(FILEGLOB_NEED_FILETIME_TO_TIME_T_CONVERSION)
time_t davglob_ConvertToTime_t(const FILETIME* fileTime);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __DAVGLOB_H__ */
