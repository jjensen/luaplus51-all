#ifndef __STRUTL_H__
#define __STRUTL_H__

char *wd_strndup(const char *s, size_t len);
char *wd_strnunqdup(const char *s, size_t len);
int wd_strchrpos(const char *s, int c);
int wd_strchrqpos(const char *s, int c);
void wd_strclrws(const char **s);
char *wd_strdup_url_encoded(const char *string);
char *wd_strdup_url_decoded(const char *string);
char *wd_strdup_base64(const char *string);

#if defined(_WIN32)
#define strcasecmp	_stricmp
#define strncasecmp _strnicmp
#endif

char *wd_strdup(const char *s);

#endif
