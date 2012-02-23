#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#define TRUE			1
#define FALSE			0

int 
get_int(const char *string, int length)
{
	int i, result = 0;
	for(i = 0; i < length; i++)
	{
		result = (result * 10) + (string[i] - '0');
	}
	return result;
}

int
scan_year_month_day(const char *string, struct tm *date)
{
	/* 2000-12-31 
	   0123456789 */
	if((isdigit(string[0]) && isdigit(string[1]) && isdigit(string[2]) && isdigit(string[3]))
			&& (string[4] == '-' || string[4] == '/')
			&& (isdigit(string[5]) && isdigit(string[6]))
			&& (string[7] == '-' || string[7] == '/')
			&& (isdigit(string[8]) && isdigit(string[9])))
	{
		date->tm_year = get_int(&string[0], 4) - 1900;
		date->tm_mon = get_int(&string[5], 2) - 1;
		date->tm_mday = get_int(&string[8], 2);
		return 10;
	}
	return 0;
}

int
scan_day_month_year(const char *string, struct tm *date)
{
	/* 31/12/2000
	   0123456789 */
	if((isdigit(string[0]) && isdigit(string[1]))
			&& (string[2] == '-' || string[2] == '/')
			&& (isdigit(string[3]) && isdigit(string[4]))
			&& (string[5] == '-' || string[5] == '/')
			&& (isdigit(string[6]) && isdigit(string[7]) && isdigit(string[8]) && isdigit(string[9])))
	{
		date->tm_mday = get_int(&string[0], 2);
		date->tm_mon = get_int(&string[3], 2) - 1;
		date->tm_year = get_int(&string[6], 4) - 1900;
		return 10;
	}
	return 0;
}

int
get_month(const char *name)
{
	static const char *month_names[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	int i;
	for(i = 0; i < 12; i++)
	{
		if((month_names[i][0] == name[0]) && (month_names[i][1] == name[1]) && (month_names[i][2] == name[2]))
		{
			return i;
		}
	}
	return 0;
}

int
scan_day_month_name_year(const char *string, struct tm *date)
{
	/* 31 Dec 2000
	   01234567890 */
	if((isdigit(string[0]) && isdigit(string[1]))
			&& (string[2] == ' ')
			&& (isalpha(string[3]) && isalpha(string[4]) && isalpha(string[5]))
			&& (string[6] == ' ')
			&& (isdigit(string[7]) && isdigit(string[8]) && isdigit(string[9]) && isdigit(string[10])))
	{
		date->tm_mday = get_int(&string[0], 2);
		date->tm_mon = get_month(&string[3]);
		date->tm_year = get_int(&string[7], 4) - 1900;
		return 11;
	}
	return 0;
}

int
scan_hour_min_sec(const char *string, struct tm *date)
{
	/* 24:59:59 
	   01234567 */
	if((isdigit(string[0]) && isdigit(string[1]))
			&& (string[2] == ':' || string[2] == '.')
			&& (isdigit(string[3]) && isdigit(string[4]))
			&& (string[5] == ':' || string[5] == '.')
			&& (isdigit(string[6]) && isdigit(string[7])))
	{
		date->tm_hour = get_int(&string[0], 2);
		date->tm_min = get_int(&string[3], 2);
		date->tm_sec = get_int(&string[6], 2);
		return 8;
	}
	return 0;
}

int
scan_time_zone_offset(const char *string, int *second_offset)
{
	int hour_offset, min_offset;
	/* +08:00
	   012345 */
	if((string[0] == '+' || string[0] == '-')
			&& (isdigit(string[1]) && isdigit(string[2]))
			&& (string[3] == ':' || string[3] == '.')
			&& (isdigit(string[4]) && isdigit(string[5])))
	{
		hour_offset = get_int(&string[1], 2);
		min_offset = get_int(&string[4], 2);
		*second_offset = ((hour_offset * 60) + min_offset) * 60;
		if(string[0] == '+')
		{
			*second_offset = - *second_offset;
		}
		return 6;
	}
	return 0;
}

time_t
get_time_from_string(const char *string)
{
	int n, second_offset = 0;
	int scanned_date = FALSE, scanned_time = FALSE, scanned_tz = FALSE;
	const char *p = string;
	struct tm date;
	time_t t;
	memset(&date, 0, sizeof(struct tm));
	while(*p != '\0')
	{
		if(!scanned_time && (n = scan_hour_min_sec(p, &date)) > 0)
		{
			scanned_time = TRUE;
		}
		else if(!scanned_date && (n = scan_year_month_day(p, &date)) > 0)
		{
			scanned_date = TRUE;
		}
		else if(!scanned_date && (n = scan_day_month_year(p, &date)) > 0)
		{
			scanned_date = TRUE;
		}
		else if(!scanned_date && (n = scan_day_month_name_year(p, &date)) > 0)
		{
			scanned_date = TRUE;
		}
		else if(!scanned_tz && (n = scan_time_zone_offset(p, &second_offset)) > 0)
		{
			scanned_tz = TRUE;
		}
		else
		{
			n = 1;
		}
		p += n;
	}
	t = mktime(&date);
	return t + second_offset;
}
