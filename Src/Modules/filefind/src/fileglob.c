/**
    Portions of this file come from the Ruby source code's dir.c and encoding.c and
    in some cases are heavily modified by Joshua Jensen.  Those portions of code fall under
    the BSDL license distributed with the Ruby source code and reproduced below:

    Copyright (C) 1993-2013 Yukihiro Matsumoto. All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
**/

#if defined(_WIN32)  &&  !defined(MINGW)
#include <windows.h>
#else
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#endif
#include <time.h>
#include "buffer.h"

static void* fileglob_DefaultAllocFunction(void* userData, void* ptr, unsigned int size) {
	(void)userData;

	if (size == 0) {
		free(ptr);
		return NULL;
	} else {
		return realloc(ptr, size);
	}
}



#define FILEGLOB_BUILD_IMPLEMENTATION

typedef struct fileglob_StringNode {
	struct fileglob_StringNode* next;
	char buffer[1];
} fileglob_StringNode;


typedef void* (*fileglob_Alloc)(void* userData, void* ptr, unsigned int size);


enum answer {UNKNOWN = -1, NO, YES};

#if !defined(_WIN32)  ||  defined(MINGW)
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#endif

typedef struct fileglob_context {
	struct fileglob_context* prev;

    char *buf;
	const char *path;
	long pathlen;
    const char *savepath;
	int dirsep; /* '/' should be placed before appending child entry's name to 'path'. */
	enum answer exist; /* Does 'path' indicate an existing entry? */
	enum answer isdir; /* Does 'path' indicate a directory or a symlink to a directory? */
    struct glob_pattern **beg;
    struct glob_pattern **end;

	struct glob_pattern **new_beg;
    struct glob_pattern **copy_beg;
	int plain, magical, recursive, match_files, match_dir;
    int goto_top_continue;

#if defined(_WIN32)  &&  !defined(MINGW)
	WIN32_FIND_DATA fd;
	HANDLE handle;
#else
	DIR* dirp;
	struct dirent* dp;
	struct stat attr;
#endif
} fileglob_context;


typedef struct fileglob {
	fileglob_Alloc allocFunction;
	void* userData;

	int flags;
	struct glob_pattern *list;
	char *buf;

	fileglob_context* context;
	fileglob_context* startingContext;

	fileglob_StringNode* exclusiveDirectoryPatternsHead;
	fileglob_StringNode* exclusiveDirectoryPatternsTail;

	fileglob_StringNode* exclusiveFilePatternsHead;
	fileglob_StringNode* exclusiveFilePatternsTail;

	fileglob_StringNode* ignoreDirectoryPatternsHead;
	fileglob_StringNode* ignoreDirectoryPatternsTail;

	fileglob_StringNode* ignoreFilePatternsHead;
	fileglob_StringNode* ignoreFilePatternsTail;

	int filesAndFolders;

#if defined(_WIN32)  &&  !defined(MINGW)
    WIN32_FIND_DATA* curfd;
#else
    struct stat* curattr;
#endif
	char permissions[10];
} fileglob;


#include "fileglob.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define MODIFIER_CHARACTER '@'

#define FILEGLOB_BUILD_IMPLEMENTATION

#if defined(_WIN32)  &&  !defined(MINGW)

BOOL (WINAPI *fnSystemTimeToTzSpecificLocalTime)(LPTIME_ZONE_INFORMATION lpTimeZone, LPSYSTEMTIME lpUniversalTime, LPSYSTEMTIME lpLocalTime);

time_t fileglob_ConvertToTime_t(const FILETIME* fileTime) {
	SYSTEMTIME universalSystemTime;
	SYSTEMTIME sysTime;
	struct tm atm;

	FileTimeToSystemTime(fileTime, &universalSystemTime);

	if (!fnSystemTimeToTzSpecificLocalTime) {
		HMODULE aLib = LoadLibraryA("kernel32.dll");
		if (aLib == NULL)
			return 0;

		*(void**)&fnSystemTimeToTzSpecificLocalTime = (void*)GetProcAddress(aLib, "SystemTimeToTzSpecificLocalTime");
	}
	fnSystemTimeToTzSpecificLocalTime(NULL, &universalSystemTime, &sysTime);

	// then convert the system time to a time_t (C-runtime local time)
	if (sysTime.wYear < 1900) {
		return 0;
	}

	atm.tm_sec = sysTime.wSecond & ~1;		// Zip files are only accurate to 2 seconds.
	atm.tm_min = sysTime.wMinute;
	atm.tm_hour = sysTime.wHour;
	atm.tm_mday = sysTime.wDay;
	atm.tm_mon = sysTime.wMonth - 1;        // tm_mon is 0 based
	atm.tm_year = sysTime.wYear - 1900;     // tm_year is 1900 based
	atm.tm_isdst = -1;
	return mktime(&atm);
}

#endif


static void _fileglob_list_clear(fileglob* self, fileglob_StringNode** head, fileglob_StringNode** tail) {
	fileglob_StringNode* node;
	for (node = *head; node;) {
		fileglob_StringNode* oldNode = node;
		node = node->next;
		self->allocFunction(self->userData, oldNode, 0);
	}

	*head = 0;
	*tail = 0;
}


static void _fileglob_list_append(fileglob* self, fileglob_StringNode** head, fileglob_StringNode** tail, const char* theString) {
	size_t patternLen = strlen(theString);
	fileglob_StringNode* newNode = (fileglob_StringNode*)self->allocFunction(self->userData, NULL, sizeof(fileglob_StringNode) + patternLen);
	if (!*head)
		*head = newNode;
	if (*tail)
		(*tail)->next = newNode;
	*tail = newNode;
	newNode->next = NULL;
	memcpy(newNode->buffer, theString, patternLen + 1);
}


/**
	\author Jack Handy

	Borrowed from http://www.codeproject.com/string/wildcmp.asp.
	Modified by Joshua Jensen.
**/
int fileglob_WildMatch(const char* pattern, const char *string, int caseSensitive, int recursive) {
	const char* mp = 0;
	const char* cp = 0;

	if (caseSensitive) {
        if (recursive) {
topa:
            // Handle all the letters of the pattern and the string.
            while (*string != 0  &&  *pattern != '*') {
                if (*pattern != '?') {
                    if (toupper(*pattern) != toupper(*string))
                        return 0;
                }

                pattern++;
                string++;
            }

            while (*string != 0) {
                if (*pattern == '*') {
                    // It's a match if the wildcard is at the end.
                    ++pattern;
                    if (*pattern == '*') {
                        if (pattern[1] == '/')
                            pattern += 2;
                        while (*string != 0) {
                            while (*string != 0) {
                                if (*string == '/') {
                                    ++string;
                                    break;
                                }
                                ++string;
                            }
                            if (fileglob_WildMatch(pattern, string, caseSensitive, 1)) {
                                return 1;
                            }
                        }
                    }

                    mp = pattern;
                    cp = string + 1;
                } else {
                    if (*string == '/') {
                        if (*pattern != '/') {
                            return 0;
                        } else {
                            pattern++;
                            string++;
                            if (*pattern == 0  &&  *string != 0) {
                                return 0;
                            }
                            goto topa;
                        }
                    }
                    if (toupper(*pattern) == toupper(*string)  ||  *pattern == '?') {
                        pattern++;
                        string++;
                    } else  {
                        pattern = mp;
                        string = cp++;
                    }
                }
            }
		} else {
            // Handle all the letters of the pattern and the string.
            while (*string != 0  &&  *pattern != '*') {
                if (*pattern != '?') {
                    if (*pattern != *string)
                        return 0;
                }

                pattern++;
                string++;
            }

            while (*string != 0) {
                if (*pattern == '*') {
                    // It's a match if the wildcard is at the end.
                    if (*++pattern == 0) {
                        return 1;
                    }

                    mp = pattern;
                    cp = string + 1;
                } else {
                    if (*pattern == *string  ||  *pattern == '?') {
                        pattern++;
                        string++;
                    } else  {
                        pattern = mp;
                        string = cp++;
                    }
                }
            }
		}
	} else {
        if (recursive) {
topb:
            // Handle all the letters of the pattern and the string.
            while (*string != 0  &&  *pattern != '*') {
                if (*pattern != '?') {
                    if (toupper(*pattern) != toupper(*string))
                        return 0;
                }

                pattern++;
                string++;
            }

            while (*string != 0) {
                if (*pattern == '*') {
                    // It's a match if the wildcard is at the end.
                    ++pattern;
                    if (*pattern == '*') {
                        if (pattern[1] == '/')
                            pattern += 2;
                        while (*string != 0) {
                            if (fileglob_WildMatch(pattern, string, caseSensitive, 1)) {
                                return 1;
                            }
                            while (*string != 0) {
                                if (*string == '/') {
                                    ++string;
                                    break;
                                }
                                ++string;
                            }
                        }
                    }

                    mp = pattern;
                    cp = string + 1;
                } else {
                    if (*string == '/') {
                        if (*pattern != '/') {
                            return 0;
                        } else {
                            pattern++;
                            string++;
                            if (*pattern == 0  &&  *string != 0) {
                                return 0;
                            }
                            goto topb;
                        }
                    }
                    if (toupper(*pattern) == toupper(*string)  ||  *pattern == '?') {
                        pattern++;
                        string++;
                    } else  {
                        pattern = mp;
                        string = cp++;
                    }
                }
            }
		} else {
            // Handle all the letters of the pattern and the string.
            while (*string != 0  &&  *pattern != '*') {
                if (*pattern != '?') {
                    if (toupper(*pattern) != toupper(*string))
                        return 0;
                }

                pattern++;
                string++;
            }

            while (*string != 0) {
                if (*pattern == '*') {
                    // It's a match if the wildcard is at the end.
                    if (*++pattern == 0) {
                        return 1;
                    }

                    mp = pattern;
                    cp = string + 1;
                } else {
                    if (toupper(*pattern) == toupper(*string)  ||  *pattern == '?') {
                        pattern++;
                        string++;
                    } else {
                        pattern = mp;
                        string = cp++;
                    }
                }
            }
		}
	}

	while (*pattern == '*')
		pattern++;

	return !*pattern  &&  !*string;
}


/* Forward declares. */
#define GLOB_ALLOC(type) ((type *)self->allocFunction(self->userData, NULL, sizeof(type)))
#define GLOB_ALLOC_N(type, n) ((type *)self->allocFunction(self->userData, NULL, sizeof(type) * (n)))
#define GLOB_FREE(ptr) self->allocFunction(self->userData, (void*)ptr, 0)

static void glob_free_pattern(fileglob *self, struct glob_pattern *list);

/**
**/
static void _fileglob_FreeContextLevel(fileglob* self) {
	if (self->context) {
		fileglob_context* oldContext = self->context;
#if defined(_WIN32)  &&  !defined(MINGW)
        self->curfd = NULL;
#else
        self->curattr = NULL;
#endif
        GLOB_FREE(oldContext->buf);
        GLOB_FREE(oldContext->new_beg);
        GLOB_FREE(oldContext->copy_beg);
        if (oldContext->savepath)
            GLOB_FREE(oldContext->path);
		self->context = oldContext->prev;
#if defined(_WIN32)  &&  !defined(MINGW)
		if (oldContext->handle != INVALID_HANDLE_VALUE) {
			FindClose(oldContext->handle);
		}
#else
		if (oldContext->dirp) {
			closedir(oldContext->dirp);
		}
#endif
		self->allocFunction(self->userData, oldContext, 0);
	}
}


/**
**/
static fileglob_context* _fileglob_AllocateContextLevel(fileglob* self) {
	fileglob_context* context = self->allocFunction(self->userData, 0, sizeof(fileglob_context));
    context->prev = self->context;
#if defined(_WIN32)  &&  !defined(MINGW)
    context->handle = INVALID_HANDLE_VALUE;
#else
    context->dirp = NULL;
#endif
    context->savepath = NULL;
    context->plain = context->magical = context->recursive = context->match_files = context->match_dir = 0;
    context->goto_top_continue = 0;

    return context;
}


/**
**/
static void _fileglob_Reset(fileglob* self) {
	_fileglob_list_clear(self, &self->exclusiveDirectoryPatternsHead, &self->exclusiveDirectoryPatternsTail);
	_fileglob_list_clear(self, &self->exclusiveFilePatternsHead, &self->exclusiveFilePatternsTail);
	_fileglob_list_clear(self, &self->ignoreDirectoryPatternsHead, &self->ignoreDirectoryPatternsTail);
	_fileglob_list_clear(self, &self->ignoreFilePatternsHead, &self->ignoreFilePatternsTail);

	while (self->context) {
		_fileglob_FreeContextLevel(self);
	}

    glob_free_pattern(self, self->list);
}



/**
	Adds a pattern to the file glob database of exclusive patterns.  If any
	exclusive patterns are registered, the ignore database is completely
	ignored.  Only patterns matching the exclusive patterns will be
	candidates for matching.

	\param name The exclusive pattern.
**/
void fileglob_AddExclusivePattern(fileglob* self, const char* pattern) {
	fileglob_StringNode* node;

	if (pattern[strlen(pattern) - 1] == '/') {
		for (node = self->exclusiveDirectoryPatternsHead; node; node = node->next) {
#if defined(_WIN32)
			if (stricmp(node->buffer, pattern) == 0) {
#else
			if (strcasecmp(node->buffer, pattern) == 0) {
#endif
				return;
            }
		}

		_fileglob_list_append(self, &self->exclusiveDirectoryPatternsHead, &self->exclusiveDirectoryPatternsTail, pattern);
	} else {
		for (node = self->exclusiveFilePatternsHead; node; node = node->next) {
#if defined(_WIN32)
			if (stricmp(node->buffer, pattern) == 0) {
#else
			if (strcasecmp(node->buffer, pattern) == 0) {
#endif
				return;
            }
		}

		_fileglob_list_append(self, &self->exclusiveFilePatternsHead, &self->exclusiveFilePatternsTail, pattern);
	}
}


/**
	Adds a pattern to ignore to the file glob database.  If a pattern of
	the given name is found, its contents will not be considered for further
	matching.  The result is as if the pattern did not exist for the search
	in the first place.

	\param name The pattern to ignore.
**/
void fileglob_AddIgnorePattern(fileglob* self, const char* pattern) {
	fileglob_StringNode* node;

	if (pattern[strlen(pattern) - 1] == '/') {
		for (node = self->ignoreDirectoryPatternsHead; node; node = node->next) {
#if defined(_WIN32)
			if (stricmp(node->buffer, pattern) == 0) {
#else
			if (strcasecmp(node->buffer, pattern) == 0) {
#endif
				return;
            }
		}

		_fileglob_list_append(self, &self->ignoreDirectoryPatternsHead, &self->ignoreDirectoryPatternsTail, pattern);
	} else {
		for (node = self->ignoreFilePatternsHead; node; node = node->next) {
#if defined(_WIN32)
			if (stricmp(node->buffer, pattern) == 0) {
#else
			if (strcasecmp(node->buffer, pattern) == 0) {
#endif
				return;
            }
		}

		_fileglob_list_append(self, &self->ignoreFilePatternsHead, &self->ignoreFilePatternsTail, pattern);
	}
}


/**
	Match an exclusive pattern.

	\param text The text to match an exclusive pattern against.
	\return Returns true if the directory should be ignored, false otherwise.
**/
static int _fileglob_MatchExclusiveDirectoryPattern(fileglob* self, const char* text) {
	fileglob_StringNode* node;

	for (node = self->exclusiveDirectoryPatternsHead; node; node = node->next) {
		if (fileglob_WildMatch(node->buffer, text, 0, 1))
			return 1;
	}

	return 0;
}


/**
	Match an exclusive pattern.

	\param text The text to match an exclusive pattern against.
	\return Returns true if the directory should be ignored, false otherwise.
**/
static int _fileglob_MatchExclusiveFilePattern(fileglob* self, const char* text) {
	fileglob_StringNode* node;

	for (node = self->exclusiveFilePatternsHead; node; node = node->next) {
		if (fileglob_WildMatch(node->buffer, text, 0, 1))
			return 1;
	}

	return 0;
}


/**
	Do a case insensitive find for the pattern.

	\param text The text to match an ignore pattern against.
	\return Returns true if the directory should be ignored, false otherwise.
**/
static int _fileglob_MatchIgnoreDirectoryPattern(fileglob* self, const char* text) {
	fileglob_StringNode* node;

	for (node = self->ignoreDirectoryPatternsHead; node; node = node->next) {
		if (fileglob_WildMatch(node->buffer, text, 0, 1))
			return 1;
	}

	return 0;
}


/**
	Do a case insensitive find for the pattern.

	\param text The text to match an ignore pattern against.
	\return Returns true if the directory should be ignored, false otherwise.
**/
static int _fileglob_MatchIgnoreFilePattern(fileglob* self, const char* text) {
	fileglob_StringNode* node;

	for (node = self->ignoreFilePatternsHead; node; node = node->next) {
		if (fileglob_WildMatch(node->buffer, text, 0, 1))
			return 1;
	}

	return 0;
}


#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FNM_CASEFOLD	0x08
#if CASEFOLD_FILESYSTEM
#define FNM_SYSCASE	FNM_CASEFOLD
#else
#define FNM_SYSCASE	0
#endif

int rb_enc_mbclen(const char *p, const char *e) {
    return 1;
}


# define Next(p, e) ((p)+ rb_enc_mbclen((p), (e)))
# define Inc(p, e) ((p) = Next((p), (e)))


/* Return nonzero if S has any special globbing chars in it.  */
static int has_magic(const char *p, const char *pend, int flags) {
	const int nocase = flags & FNM_CASEFOLD;

	register char c;

	while (p < pend && (c = *p++) != 0) {
		switch (c) {
		case '*':
		case '?':
		case '[':
			return 1;

		default:
			if (!FNM_SYSCASE && isalpha(c) && nocase)
				return 1;
		}

		p = Next(p-1, pend);
	}

	return 0;
}


/* Find separator in globbing pattern. */
static char *find_dirsep(const char *p, const char *pend, int flags) {
	register char c;

	while ((c = *p++) != 0) {
		switch (c) {
            case '\\':
            case '/':
                return (char *)p-1;
		}

		p = Next(p-1, pend);
	}

	return (char *)p-1;
}


/* Globing pattern */
enum glob_pattern_type { PLAIN, MAGICAL, RECURSIVE, MATCH_ALL, MATCH_FILES, MATCH_DIR };

struct glob_pattern {
	char *str;
	enum glob_pattern_type type;
	struct glob_pattern *next;
};

static struct glob_pattern *glob_make_pattern(fileglob *self, const char *p, const char *e, int flags) {
	struct glob_pattern *list, *tmp, **tail = &list;
	int dirsep = 0; /* pattern is terminated with '/' */
	int recursive = 0;

	while (p < e && *p) {
		tmp = GLOB_ALLOC(struct glob_pattern);
		if (!tmp) goto error;
		if (p[0] == '*' && p[1] == '*' && p[2] == '/') {
			/* fold continuous RECURSIVEs (needed in glob_helper) */
			do { p += 3; while (*p == '/') p++; } while (p[0] == '*' && p[1] == '*' && p[2] == '/');
			tmp->type = RECURSIVE;
			tmp->str = 0;
			dirsep = 1;
			recursive = 1;
		}
		else {
			const char *m = find_dirsep(p, e, flags);
			int magic = has_magic(p, m, flags);
			char *buf;

			if (!magic && !recursive && *m) {
				const char *m2;
				while (!has_magic(m+1, m2 = find_dirsep(m+1, e, flags), flags) &&
					*m2) {
						m = m2;
				}
			}
			buf = GLOB_ALLOC_N(char, m-p+1);
			if (!buf) {
				GLOB_FREE(tmp);
				goto error;
			}
			memcpy(buf, p, m-p);
			buf[m-p] = '\0';
			tmp->type = magic ? MAGICAL : PLAIN;
			tmp->str = buf;
			if (*m) {
				dirsep = 1;
				p = m + 1;
			}
			else {
				dirsep = 0;
				p = m;
			}
		}
		*tail = tmp;
		tail = &tmp->next;
	}

	tmp = GLOB_ALLOC(struct glob_pattern);
	if (!tmp) {
error:
		*tail = 0;
		glob_free_pattern(self, list);
		return 0;
	}
	tmp->type = self->filesAndFolders ? MATCH_ALL : (dirsep ? MATCH_DIR : MATCH_FILES);
	tmp->str = 0;
	*tail = tmp;
	tmp->next = 0;

	return list;
}


static void glob_free_pattern(fileglob* self, struct glob_pattern *list) {
	while (list) {
		struct glob_pattern *tmp = list;
		list = list->next;
		if (tmp->str)
			GLOB_FREE(tmp->str);
		GLOB_FREE(tmp);
	}
}


static char *join_path(fileglob *self, const char *path, long len, int dirsep, const char *name, size_t namlen) {
	char *buf = GLOB_ALLOC_N(char, len+namlen+(dirsep?1:0)+1);

	if (!buf) return 0;
	memcpy(buf, path, len);
	if (dirsep) {
		buf[len++] = '/';
	}
	memcpy(buf+len, name, namlen);
	buf[len+namlen] = '\0';
	return buf;
}


/**
	Finds all files matching [inPattern].  The wildcard expansion understands
	the following pattern constructs:

	- ?
		- Any single character.
	- *
		- Any character of which there are zero or more of them.
	- **
		- All subdirectories recursively.

	Additionally, if the pattern closes in a single slash, only directories
	are processed.  Forward slashes and backslashes may be used
	interchangeably.

	- *\
		- List all the directories in the current directory.  Can also be
		  *(forwardslash), but it turns off the C++ comment in this message.
	- **\
		- List all directories recursively.

	Wildcards may appear anywhere in the pattern, including directories.

	- *\*\*\*.c

	Note that *.* only matches files that have an extension.  This is different
	than standard DOS behavior.  Use * all by itself to match files, extension
	or not.

	Recursive wildcards can be used anywhere:

	c:/Dir1\**\A*\**\FileDirs*\**.mp3

	This matches all directories under c:\Dir1\ that start with A.  Under all
	of the directories that start with A, directories starting with FileDirs
	are matched recursively.  Finally, all files ending with an mp3 extension
	are matched.

	Any place that has more than two .. for going up a subdirectory is expanded
	a la 4DOS.

	...../StartSearchHere\**

	Expands to:

	../../../../StartSearchHere\**

	\param inPattern The pattern to use for matching.
**/
void fileglob_MatchPattern(fileglob* self, const char* inPattern, BUFFER* destBuff) {
	const char* srcPtr;
	const char* lastSlashPtr;
	int numPeriods;

	buffer_initwithalloc(destBuff, self->allocFunction, self->userData);

	if (inPattern == 0)
		inPattern = "*";

	// Give ourselves a local copy of the inPattern with all \ characters
	// changed to / characters and more than two periods expanded.
	srcPtr = inPattern;

	// Is it a Windows network path?   If so, don't convert the opening \\.
	if (srcPtr[0] == '\\'   &&   srcPtr[1] == '\\')
	{
		buffer_addchar(destBuff, *srcPtr++);
		buffer_addchar(destBuff, *srcPtr++);
	}

	lastSlashPtr = srcPtr - 1;
	numPeriods = 0;
	while (*srcPtr != '\0') {
		char ch = *srcPtr;

		///////////////////////////////////////////////////////////////////////
		// Check for slashes or backslashes.
		if (ch == '\\'  ||  ch == '/') {
			buffer_addchar(destBuff, '/');

			lastSlashPtr = srcPtr;
			numPeriods = 0;
		}

		///////////////////////////////////////////////////////////////////////
		// Check for .
		else if (ch == '.') {
			if (srcPtr - numPeriods - 1 == lastSlashPtr) {
				numPeriods++;
				if (numPeriods > 2) {
					buffer_addchar(destBuff, '/');
					buffer_addchar(destBuff, '.');
					buffer_addchar(destBuff, '.');
				} else {
					buffer_addchar(destBuff, '.');
				}
			} else {
				buffer_addchar(destBuff, '.');
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Check for **
		else if (ch == '*'  &&  srcPtr[1] == '*') {
			if (srcPtr - 1 != lastSlashPtr) {
				// Something like this:
				//
				// /Dir**/
				//
				// needs to be translated to:
				//
				// /Dir*/**/
				buffer_addchar(destBuff, '*');
				buffer_addchar(destBuff, '/');
			}

			srcPtr += 2;

			buffer_addchar(destBuff, '*');
			buffer_addchar(destBuff, '*');

			// Did we get a double star this round?
			if (srcPtr[0] != '/'  &&  srcPtr[0] != '\\') {
				// Handle the case that looks like this:
				//
				// /**Text
				//
				// Translate to:
				//
				// /**/*Text
				buffer_addchar(destBuff, '/');
				buffer_addchar(destBuff, '*');
			}
			else if (srcPtr[1] == '\0'  ||  srcPtr[1] == MODIFIER_CHARACTER) {
				srcPtr++;

				buffer_addchar(destBuff, '/');
				buffer_addchar(destBuff, '*');
				buffer_addchar(destBuff, '/');
			}

			// We added one too many in here... the compiler will optimize.
			srcPtr--;
		}

		///////////////////////////////////////////////////////////////////////
		// Check for @
		else if (ch == MODIFIER_CHARACTER) {
			// Gonna finish this processing in another loop.
			break;
		}

		///////////////////////////////////////////////////////////////////////
		// Everything else.
		else {
			buffer_addchar(destBuff, *srcPtr);
		}

		srcPtr++;
	}

	buffer_addchar(destBuff, 0);

	// Check for the @.
	if (*srcPtr == MODIFIER_CHARACTER) {
		_fileglob_Reset(self);
	}

	while (*srcPtr == MODIFIER_CHARACTER) {
		char ch;

		srcPtr++;

		ch = *srcPtr++;

		///////////////////////////////////////////////////////////////////////
		// Check for @- or @=
		if (ch == '-'  ||  ch == '=') {
			BUFFER buff;
			buffer_initwithalloc(&buff, self->allocFunction, self->userData);
			while (*srcPtr != MODIFIER_CHARACTER  &&  *srcPtr != '\0') {
				buffer_addchar(&buff, *srcPtr++);
			}

			buffer_addchar(&buff, 0);

			if (ch == '-')
				fileglob_AddIgnorePattern(self, buffer_ptr(&buff));
			else if (ch == '=')
				fileglob_AddExclusivePattern(self, buffer_ptr(&buff));
			buffer_free(&buff);

		///////////////////////////////////////////////////////////////////////
		// Check for @*
		} else if (ch == '*') {
			self->filesAndFolders = 1;
			while (*srcPtr != MODIFIER_CHARACTER  &&  *srcPtr != '\0') {
				++srcPtr;
			}

		///////////////////////////////////////////////////////////////////////
		// Anything else is ignored.
		} else {
			// Don't know what it is.
			while (*srcPtr != MODIFIER_CHARACTER  &&  *srcPtr != '\0') {
				++srcPtr;
			}
		}
	}
}


/**
**/
fileglob* fileglob_CreateWithAlloc(const char* inPattern, fileglob_Alloc allocFunction, void* userData) {
	fileglob* self;
	const char *root, *start;
	size_t n;
    int flags = 0;
    fileglob_context* context;
    BUFFER inPatternBuf;

	allocFunction = allocFunction ? allocFunction : fileglob_DefaultAllocFunction;
	self = (fileglob*)allocFunction(userData, NULL, sizeof(fileglob));
	memset(self, 0, sizeof(fileglob));
	self->allocFunction = allocFunction;
	self->userData = userData;

	fileglob_MatchPattern(self, inPattern, &inPatternBuf);
	start = root = buffer_ptr(&inPatternBuf);
	flags |= FNM_SYSCASE;

	if (root && *root == '/') root++;

	n = root - start;
	self->buf = GLOB_ALLOC_N(char, n + 1);
	if (!self->buf) return NULL; //-1;
	memcpy(self->buf, start, n);
	self->buf[n] = '\0';

	self->list = glob_make_pattern(self, root, root + strlen(root), flags);
	if (!self->list) {
		GLOB_FREE(self->buf);
		return NULL; //-1;
	}
    self->context = NULL;
    self->startingContext = _fileglob_AllocateContextLevel(self);
    context = self->startingContext;
    context->buf = self->buf;
    context->path = self->buf;
    context->pathlen = strlen(self->buf);
    context->beg = &self->list;
    context->end = &self->list + 1;
	context->new_beg = NULL;
    context->copy_beg = NULL;

    context->dirsep = 0;
	context->exist = UNKNOWN;
    context->isdir = UNKNOWN;

	return self;
}


/**
**/
fileglob* fileglob_Create(const char* inPattern) {
	return fileglob_CreateWithAlloc(inPattern, NULL, NULL);
}


/**
**/
int fileglob_Next(fileglob* self) {
	struct glob_pattern **cur;

	fileglob_context* context;

    if (self->context) {
        if (self->context->goto_top_continue) {
            context = self->context;
			GLOB_FREE(context->path);
            context->path = context->savepath;
            context->savepath = NULL;
            context->goto_top_continue = 0;
            goto TopContinue;
		}
        goto NextContext;
	}

    self->context = self->startingContext;
    if (!self->context) {
        return 0;
	}
    self->startingContext = NULL;

Top:
	context = self->context;

	for (cur = context->beg; cur < context->end; ++cur) {
		struct glob_pattern *p = *cur;
		if (p->type == RECURSIVE) {
			context->recursive = 1;
			p = p->next;
		}
		switch (p->type) {
            case PLAIN:
                context->plain = 1;
                break;
            case MAGICAL:
                context->magical = 1;
                break;
            case MATCH_ALL:
                context->match_dir = 1;
                context->match_files = 1;
                break;
            case MATCH_FILES:
                context->match_files = 1;
                break;
            case MATCH_DIR:
                context->match_dir = 1;
                break;
            case RECURSIVE:
                //rb_bug("continuous RECURSIVEs");
                break;
		}
	}

	if (*context->path) {
		if (context->match_files && context->exist == UNKNOWN) {
#if defined(_WIN32)  &&  !defined(MINGW)
            HANDLE handle = FindFirstFile(context->path, &context->fd);
            if (handle != INVALID_HANDLE_VALUE) {
                FindClose(handle);
                self->curfd = &context->fd;
				context->exist = YES;
				context->isdir = (context->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? YES : NO;
			} else {
				context->exist = NO;
				context->isdir = NO;
			}
#else
			if (stat(context->path, &context->attr) == 0) {
                self->curattr = &context->attr;
				context->exist = YES;
				context->isdir = S_ISDIR(context->attr.st_mode) ? YES : NO;
			}
			else {
				context->exist = NO;
				context->isdir = NO;
			}
#endif
		}
		if (context->match_dir && context->isdir == UNKNOWN) {
#if defined(_WIN32)  &&  !defined(MINGW)
            HANDLE handle = FindFirstFile(context->path, &context->fd);
            if (handle != INVALID_HANDLE_VALUE) {
                FindClose(handle);
                self->curfd = &context->fd;
				context->exist = YES;
				context->isdir = (context->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? YES : NO;
			} else {
				context->exist = NO;
				context->isdir = NO;
			}
#else
			if (stat(context->path, &context->attr) == 0) {
                self->curattr = &context->attr;
				context->exist = YES;
				context->isdir = S_ISDIR(context->attr.st_mode) ? YES : NO;
			}
			else {
				context->exist = NO;
				context->isdir = NO;
			}
#endif
		}
        if (context->match_files && context->isdir != YES && context->exist == YES) {
            return 1;
		}
		if (context->match_dir && context->isdir == YES) {
            context->savepath = context->path;
			context->path = join_path(self, context->path, context->pathlen, context->dirsep, "", 0);
			//if (!tmp) return -1;
            context->goto_top_continue = 1;
            return 1;
		}
	}

TopContinue:
	if (context->exist == NO || context->isdir == NO)
		goto NextContext;

	if (context->magical || context->recursive) {
        const char* path = *context->path ? context->path : ".";
#if defined(_WIN32)  &&  !defined(MINGW)
        BUFFER wildcardBuff;
        buffer_init(&wildcardBuff);
        buffer_addstring(&wildcardBuff, path, strlen(path));
        buffer_addstring(&wildcardBuff, "\\*.*", 5);
        context->handle = FindFirstFile(buffer_ptr(&wildcardBuff), &context->fd);
        buffer_free(&wildcardBuff);
        if (context->handle == INVALID_HANDLE_VALUE)
            goto NextContext;
#else
		context->dirp = opendir(path);
        if (context->dirp == NULL) return 0;
        context->dp = readdir(context->dirp);
        if (!context->dp)
            goto NextContext;
#endif

		while (1) {
            struct glob_pattern **new_beg, **new_end;
			char *buf;
			enum answer new_isdir = UNKNOWN;
			const char *name;
			size_t namlen;

            // Knock out "." or ".."
#if defined(_WIN32)  &&  !defined(MINGW)
			name = context->fd.cFileName;
            new_isdir = (context->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? YES : NO;
			if (new_isdir  &&  (name[0] == '.'  &&  (name[1] == 0  ||  (name[1] == '.'  &&  name[2] == 0)))) {
                goto NextFile;
			}
			namlen = strlen(name);
#else
			name = context->dp->d_name;
			if (name[0] == '.'  &&  (name[1] == 0  ||  (name[1] == '.'  &&  name[2] == 0))) {
                goto NextFile;
			}
			/*namlen = context->dp->d_namlen;*/
			namlen = strlen(name);
#endif
			buf = join_path(self, context->path, context->pathlen, context->dirsep, name, namlen);
			if (!buf) {
				break;
			}
			name = buf + context->pathlen + (context->dirsep != 0);

#if !defined(_WIN32)  ||  defined(MINGW)
            memset(&context->attr, 0, sizeof(context->attr));
			if (stat(buf, &context->attr) == 0) {
				new_isdir = S_ISDIR(context->attr.st_mode) ? YES : NO;
			}
#endif

            if (new_isdir != YES) {
                int ignore = 0;
                if (self->ignoreFilePatternsHead  ||  self->exclusiveFilePatternsHead) {
                    ignore = _fileglob_MatchIgnoreFilePattern(self, buf);

                    // Is this pattern exclusive?
                    if (!ignore  &&  self->exclusiveFilePatternsHead) {
                        ignore = !_fileglob_MatchExclusiveFilePattern(self, buf);
                    }
                }

                // Should this file be ignored?
                if (ignore) {
                    GLOB_FREE(buf);
                    goto NextFile;
				}
			} else {
                int ignore = 0;
                if (self->ignoreDirectoryPatternsHead  ||  self->exclusiveDirectoryPatternsHead) {
                    char* dirbuf = join_path(self, buf, strlen(buf), context->dirsep, "", 0);

                    ignore = _fileglob_MatchIgnoreDirectoryPattern(self, dirbuf);

                    // Is this pattern exclusive?
                    if (!ignore  &&  self->exclusiveDirectoryPatternsHead) {
                        ignore = !_fileglob_MatchExclusiveDirectoryPattern(self, dirbuf);
                    }
                    GLOB_FREE(dirbuf);
                }

                // Should this directory be ignored?
                if (ignore) {
                    goto NextFile;
				}
			}

			new_beg = new_end = GLOB_ALLOC_N(struct glob_pattern *, (context->end - context->beg) * 2);
			if (!new_beg) {
				GLOB_FREE(buf);
				//status = -1;
				break;
			}

			for (cur = context->beg; cur < context->end; ++cur) {
				struct glob_pattern *p = *cur;
				if (p->type == RECURSIVE) {
					if (new_isdir == YES) /* not symlink but real directory */
						*new_end++ = p; /* append recursive pattern */
					p = p->next; /* 0 times recursion */
				}
				if (p->type == PLAIN || p->type == MAGICAL) {
                    if (fileglob_WildMatch(p->str, name, 0, 0))
						*new_end++ = p->next;
				}
			}

			context = _fileglob_AllocateContextLevel(self);
			context->buf = buf;
			context->path = context->buf;
			context->pathlen = strlen(context->buf);
			context->dirsep = 1;
			context->exist = YES;
			context->isdir = new_isdir;
			context->beg = new_beg;
			context->end = new_end;
            context->new_beg = new_beg;
            context->copy_beg = NULL;
#if defined(_WIN32)  &&  !defined(MINGW)
            self->curfd = &self->context->fd;
#else
            self->curattr = &self->context->attr;
#endif
            self->context = context;

			goto Top;

NextFile:
#if defined(_WIN32)  &&  !defined(MINGW)
			if (!FindNextFile(context->handle, &context->fd))
				break;
#else
            context->dp = readdir(context->dirp);
            if (!context->dp)
                break;
#endif
		}

#if defined(_WIN32)  &&  !defined(MINGW)
		FindClose(context->handle);
        context->handle = INVALID_HANDLE_VALUE;
#else
        closedir(context->dirp);
        context->dirp = NULL;
#endif
	}
	else if (context->plain) {
		struct glob_pattern **copy_beg, **copy_end, **cur2;
		struct glob_pattern **new_beg, **new_end;

		copy_beg = copy_end = GLOB_ALLOC_N(struct glob_pattern *, context->end - context->beg);
		if (!copy_beg) return -1;
		for (cur = context->beg; cur < context->end; ++cur)
			*copy_end++ = (*cur)->type == PLAIN ? *cur : 0;

		for (cur = copy_beg; cur < copy_end; ++cur) {
			if (*cur) {
				char *buf;
				char *name;
				size_t len = strlen((*cur)->str) + 1;
				name = GLOB_ALLOC_N(char, len);
				if (!name) {
					//status = -1;
					break;
				}
				memcpy(name, (*cur)->str, len);

				new_beg = new_end = GLOB_ALLOC_N(struct glob_pattern *, context->end - context->beg);
				if (!new_beg) {
					GLOB_FREE(name);
					break;
				}
				*new_end++ = (*cur)->next;
				for (cur2 = cur + 1; cur2 < copy_end; ++cur2) {
					//if (*cur2 && fnmatch((*cur2)->str, name, self->flags) == 0) {
                    if (*cur2 && fileglob_WildMatch((*cur2)->str, name, 0, 0)) {
						*new_end++ = (*cur2)->next;
						*cur2 = 0;
					}
				}

				buf = join_path(self, context->path, context->pathlen, context->dirsep, name, len);
				GLOB_FREE(name);
				if (!buf) {
					GLOB_FREE(new_beg);
					break;
				}

                context = _fileglob_AllocateContextLevel(self);
				context->buf = buf;
				context->path = context->buf;
				context->pathlen = strlen(context->buf);
				context->dirsep = 1;
				context->exist = UNKNOWN;
				context->isdir = UNKNOWN;
				context->beg = new_beg;
				context->end = new_end;
                context->new_beg = new_beg;
                context->copy_beg = copy_beg;
#if defined(_WIN32)  &&  !defined(MINGW)
                self->curfd = NULL;
#else
                self->curattr = NULL;
#endif
                self->context = context;

				goto Top;
			}
		}
	}

NextContext:
    if (self->context->prev) {
        _fileglob_FreeContextLevel(self);
        context = self->context;
#if defined(_WIN32)  &&  !defined(MINGW)
        if (context->handle != INVALID_HANDLE_VALUE) {
            goto NextFile;
		}
#else
        if (context->dirp != NULL) {
            goto NextFile;
		}
#endif
        if (self->context->prev) {
            goto NextContext;
		}
    }

    return 0;
}


/**
**/
void fileglob_Destroy(fileglob* self) {
	if (!self)
		return;
	_fileglob_Reset(self);
	self->allocFunction(self->userData, self, 0);
}


const char* fileglob_FileName(fileglob* self) {
#if defined(_WIN32)  &&  !defined(MINGW)
	//if (self->context->handle != INVALID_HANDLE_VALUE) {
        return self->context->path;
		//SplicePath(&self->combinedName, buffer_ptr(&self->context->basePath), self->context->fd.cFileName);
		//return buffer_ptr(&self->combinedName);
	//}
#else
        return self->context->path;
	//if (self->context->dirp) {
		//SplicePath(&self->combinedName, buffer_ptr(&self->context->basePath), self->context->dp->d_name);
		//return buffer_ptr(&self->combinedName);
	//} else {
		//return buffer_ptr(&self->context->patternBuf);
	//}
#endif

	return NULL;
}


fileglob_uint64 fileglob_CreationTime(fileglob* self) {
#if defined(_WIN32)  &&  !defined(MINGW)
	if (self->curfd) {
		return fileglob_ConvertToTime_t(&self->curfd->ftCreationTime);
	}
#else
	if (self->curattr) {
		return self->curattr->st_ctime;
	}
#endif

	return 0;
}


fileglob_uint64 fileglob_AccessTime(fileglob* self) {
#if defined(_WIN32)  &&  !defined(MINGW)
	if (self->curfd) {
		return fileglob_ConvertToTime_t(&self->curfd->ftLastAccessTime);
	}
#else
	if (self->curattr) {
		return self->curattr->st_atime;
	}
#endif

	return 0;
}


fileglob_uint64 fileglob_WriteTime(fileglob* self) {
#if defined(_WIN32)  &&  !defined(MINGW)
	if (self->curfd) {
		return fileglob_ConvertToTime_t(&self->curfd->ftLastWriteTime);
	}
#else
	if (self->curattr) {
		return self->curattr->st_mtime;
	}
#endif

	return 0;
}


fileglob_uint64 fileglob_CreationFILETIME(fileglob* self) {
#if defined(_WIN32)  &&  !defined(MINGW)
	if (self->curfd) {
		return ((fileglob_uint64)self->curfd->ftCreationTime.dwHighDateTime << 32) |
				(fileglob_uint64)self->curfd->ftCreationTime.dwLowDateTime;
	}
#else
	if (self->curattr) {
	}
#endif

	return 0;
}


fileglob_uint64 fileglob_AccessFILETIME(fileglob* self) {
#if defined(_WIN32)  &&  !defined(MINGW)
	if (self->context->handle != INVALID_HANDLE_VALUE) {
		return ((fileglob_uint64)self->context->fd.ftLastAccessTime.dwHighDateTime << 32) |
				(fileglob_uint64)self->context->fd.ftLastAccessTime.dwLowDateTime;
	}
#else
	if (self->curattr) {
	}
#endif

	return 0;
}


fileglob_uint64 fileglob_WriteFILETIME(fileglob* self) {
#if defined(_WIN32)  &&  !defined(MINGW)
	if (self->curfd) {
		return ((fileglob_uint64)self->curfd->ftLastWriteTime.dwHighDateTime << 32) |
				(fileglob_uint64)self->curfd->ftLastWriteTime.dwLowDateTime;
	}
#else
	if (self->curattr) {
	}
#endif

	return 0;
}


fileglob_uint64 fileglob_FileSize(fileglob* self) {
#if defined(_WIN32)  &&  !defined(MINGW)
	if (self->curfd) {
		return ((fileglob_uint64)self->curfd->nFileSizeLow + ((fileglob_uint64)self->curfd->nFileSizeHigh << 32));
	}
#else
	if (self->curattr) {
		return self->curattr->st_size;
	}
#endif

	return 0;
}


int fileglob_IsDirectory(fileglob* self) {
#if defined(_WIN32)  &&  !defined(MINGW)
	if (self->curfd) {
		return (self->curfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
#else
	if (self->curattr) {
		return S_ISDIR(self->curattr->st_mode);
	}
#endif

	return 0;
}


int fileglob_IsLink(fileglob* self) {
#if defined(_WIN32)  &&  !defined(MINGW)
	if (self->curfd) {
		return (self->curfd->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
	}
#else
	if (self->curattr) {
		return S_ISLNK(self->curattr->st_mode);
	}
#endif

	return 0;
}


int fileglob_IsReadOnly(fileglob* self) {
#if defined(_WIN32)  &&  !defined(MINGW)
	if (self->curfd) {
		return (self->curfd->dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
	}
#else
	if (self->curattr) {
		return (self->curattr->st_mode & S_IWUSR) == 0;
	}
#endif

	return 0;
}


const char* fileglob_Permissions(fileglob* self) {
    memset(self->permissions, '-', 9);
    self->permissions[9] = 0;
#if defined(_WIN32)  &&  !defined(MINGW)
	if (self->curfd) {
	    const char* p;
		if (self->curfd->dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
			self->permissions[0] = 'r';
			self->permissions[3] = 'r';
			self->permissions[6] = 'r';
        } else {
			self->permissions[0] = 'r';
			self->permissions[1] = 'w';
			self->permissions[3] = 'r';
			self->permissions[4] = 'w';
			self->permissions[6] = 'r';
			self->permissions[7] = 'w';
        }
        if (p = strrchr(self->context->path, '.')) {
            if ( !_stricmp(p, ".exe") ||
                !_stricmp(p, ".cmd") ||
                !_stricmp(p, ".bat") ||
                !_stricmp(p, ".com") ) {
                self->permissions[2] = 'x';
                self->permissions[5] = 'x';
                self->permissions[8] = 'x';
            }
        }
	}
#else
	if (self->curattr) {
		mode_t mode = self->curattr->st_mode;
		if (mode & S_IRUSR)
			self->permissions[0] = 'r';
		if (mode & S_IWUSR)
			self->permissions[1] = 'w';
		if (mode & S_IXUSR)
			self->permissions[2] = 'x';
		if (mode & S_IRGRP)
			self->permissions[3] = 'r';
		if (mode & S_IWGRP)
			self->permissions[4] = 'w';
		if (mode & S_IXGRP)
			self->permissions[5] = 'x';
		if (mode & S_IROTH)
			self->permissions[6] = 'r';
		if (mode & S_IWOTH)
			self->permissions[7] = 'w';
		if (mode & S_IXOTH)
			self->permissions[8] = 'x';
	}
#endif

    return self->permissions;
}


fileglob_uint64 fileglob_NumberOfLinks(fileglob* self) {
#if defined(_WIN32)  &&  !defined(MINGW)
	if (self->curfd) {
		HANDLE handle;
		BY_HANDLE_FILE_INFORMATION fileInformation;

		const char* fullPath = fileglob_FileName(self);

		handle = CreateFile(fullPath, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (handle == INVALID_HANDLE_VALUE) {
			return 0;
		}
		if (GetFileInformationByHandle(handle, &fileInformation) == FALSE) {
			CloseHandle(handle);
			return 0;
		}
		CloseHandle(handle);

		return fileInformation.nNumberOfLinks;
	}
#else
	if (self->curattr) {
		return self->curattr->st_nlink;
	}
#endif

	return 0;
}


