#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "strutl.h"

typedef void* (*http_allocator)(void *ud, void *ptr, size_t nsize);
extern http_allocator _http_allocator;
extern void* _http_allocator_user_data;

#define need_escaped(c) (((0 <= c) && (c <= 32)) || ((128 <= (unsigned char) c) && ((unsigned char) c <= 159)) || (c == '%'))
const char *hex_char = "0123456789ABCDEF";
char base64_table[] =
{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0'
};

char *
wd_strndup(const char *s, size_t len)
{
	char *new_string;
	new_string = (char *) _http_allocator(_http_allocator_user_data, 0, (len + 1) * sizeof(char));
	if(new_string != NULL)
	{
		memcpy(new_string, s, len);
		new_string[len] = '\0';
	}
	return new_string;
}

char *wd_strdup(const char *s)
{
	int length = strlen(s);
	char *new_s = _http_allocator(_http_allocator_user_data, 0, length + 1);
	if(new_s)
	{
		strcpy(new_s, s);
	}
	return new_s;
}

char *
wd_strnunqdup(const char *s, size_t len)
{
	if(s[len - 1] == '"')
	{
		len--;
	}
	if(s[0] == '"')
	{
		s++;
		len--;
	}
	return wd_strndup(s, len);
}

int
wd_strchrpos(const char *s, int c)
{
	int i;
	for(i = 0; s[i] != '\0'; i++)
	{
		if(s[i] == c)
		{
			return i;
		}
	}
	return - 1;
}

int
wd_strchrqpos(const char *s, int c)
{
	int i, quoted = 0;
	for(i = 0; s[i] != '\0'; i++)
	{
		if(!quoted)
		{
			if(s[i] == c)
			{
				return i;
			}
			else if(s[i] == '"')
			{
				quoted = 1;
			}
		}
		else
		{
			if(s[i] == '"')
			{
				quoted = 0;
			}
		}
	}
	return - 1;
}

void
wd_strclrws(const char **s)
{
	while((*s)[0] == ' ')
	{
		(*s)++;
	}
}

char *
wd_strdup_url_encoded(const char *string)
{
	int i, j, new_length = 0;
	char *new_string;
	for(i = 0; string[i] != '\0'; i++)
	{
		if(need_escaped(string[i]))
		{
			new_length += 3;
		}
		else
		{
			new_length++;
		}
	}
	new_string = (char *) _http_allocator(_http_allocator_user_data, 0, (new_length + 1) * sizeof(char));
	if(new_string == NULL)
	{
		return NULL;
	}
	for(i = 0, j = 0; string[i] != '\0'; i++)
	{
		if(need_escaped(string[i]))
		{
			new_string[j++] = '%';
			new_string[j++] = hex_char[(string[i] >> 4)];
			new_string[j++] = hex_char[(string[i] & 0x0F)];
		}
		else
		{
			new_string[j++] = string[i];
		}
	}
	new_string[j] = '\0';
	return new_string;
}

int
unhex(const char *s, int length)
{
	int c, i, value = 0;
	for(i = 0; s[i] != '\0' && i < length; i++)
	{
		c = s[i];
		if(c >= '0' && c <= '9')
		{
			value = (value << 4) | (c - '0');
		}
		else if(c >= 'a' && c <= 'z')
		{
			value = (value << 4) | (c - 'a' + 10);
		}
		else if(c >= 'A' && c <= 'Z')
		{
			value = (value << 4) | (c - 'A' + 10);
		}
	}
	return value;
}

char *
wd_strdup_url_decoded(const char *string)
{
	int i, j, new_length = 0;
	char *new_string = NULL;
	new_length = strlen(string);
	new_string = (char *) _http_allocator(_http_allocator_user_data, 0, (new_length + 1) * sizeof(char));
	if(new_string == NULL)
	{
		return NULL;
	}
	for(i = 0, j = 0; string[i] != '\0'; i++)
	{
		if(string[i] == '%')
		{
			new_string[j++] = unhex(&string[i + 1], 2);
			i += 2;
		}
		else
		{
			new_string[j++] = string[i];
		}
	}
	new_string[j] = '\0';
	return new_string;
}


char *
wd_strdup_base64(const char *string)
{
	const char *p;
	char *new_string;
	int i, j, length, new_length;
	length = strlen(string);
	new_length = (length + 3) * 4 / 3;
	new_string = (char *) _http_allocator(_http_allocator_user_data, 0, (new_length + 1) * sizeof(char));

	for(i = 0, j = length, p = string; j > 2; j -= 3, p += 3)
	{
		new_string[i++] = base64_table[p[0] >> 2];
		new_string[i++] = base64_table[((p[0] & 0x03) << 4) + (p[1] >> 4)];
		new_string[i++] = base64_table[((p[1] & 0x0f) << 2) + (p[2] >> 6)];
		new_string[i++] = base64_table[p[2] & 0x3f];
	}

	if(j != 0) 
	{
		new_string[i++] = base64_table[p[0] >> 2];
		if(j > 1) 
		{
			new_string[i++] = base64_table[((p[0] & 0x03) << 4) + (p[1] >> 4)];
			new_string[i++] = base64_table[(p[1] & 0x0f) << 2];
			new_string[i++] = '=';
		}
		else 
		{
			new_string[i++] = base64_table[(p[0] & 0x03) << 4];
			new_string[i++] = '=';
			new_string[i++] = '=';
		}
	}
	new_string[i] = '\0';
	return new_string;
}

char *
strdup_url_host(const char *url)
{
	char *double_slash = NULL, *single_slash = NULL;
	double_slash = strstr(url, "//");
	if(double_slash != NULL) 
	{
		single_slash = strchr(double_slash + 1, '/');
		if(single_slash != NULL)
		{
			return wd_strndup(double_slash + 1, strlen(double_slash + 1) - strlen(single_slash));
		}
		else
		{
			return wd_strdup(double_slash + 1);
		}
	}
	return NULL;
}

char *
strdup_url_uri(const char *url)
{
	char *double_slash = NULL, *single_slash = NULL;
	double_slash = strstr(url, "//");
	if(double_slash != NULL) 
	{
		single_slash = strchr(double_slash + 1, '/');
		if(single_slash != NULL)
		{
			return wd_strdup(single_slash);
		}
		else
		{
			return wd_strdup("/");
		}
	}
	return NULL;
}

