#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "http.h"
#include "xml_tree.h"
#include "webdav.h"
#include "strutl.h"
#include "date_decode.h"

extern http_allocator _http_allocator;
extern void* _http_allocator_user_data;

int
is_xml_node(XML_NODE *node, const char *name, const char *ns)
{
	if(node->ns == NULL && ns != NULL)
	{
		return 0;
	}
	if(strcmp(node->name, name) != 0)
	{
		return 0;
	}
	if(strcmp(node->ns, ns) != 0)
	{
		return 0;
	}
	return 1;
}

DAV_LOCKSCOPE
get_lockscope(XML_NODE *node)
{
	if(is_xml_node(node, "shared", "DAV:"))
	{
		return DAV_LOCKSCOPE_SHARED;
	}
	else if(is_xml_node(node, "exclusive", "DAV:"))
	{
		return DAV_LOCKSCOPE_EXCLUSIVE;
	}
	return DAV_LOCKSCOPE_UNKNOWN;
}

const char *
lockscope_to_str(DAV_LOCKSCOPE lockscope)
{
	if(lockscope == DAV_LOCKSCOPE_SHARED)
	{
		return "shared";
	}
	else if(lockscope == DAV_LOCKSCOPE_EXCLUSIVE)
	{
		return "exclusive";
	}
	return "";
}

DAV_LOCKTYPE
get_locktype(XML_NODE *node)
{
	if(is_xml_node(node, "write", "DAV:"))
	{
		return DAV_LOCKTYPE_WRITE;
	}
	return DAV_LOCKTYPE_UNKNOWN;
}

const char *
locktype_to_str(DAV_LOCKTYPE locktype)
{
	if(locktype == DAV_LOCKTYPE_WRITE)
	{
		return "write";
	}
	return "";
}

void
dav_lockentry_destroy(DAV_LOCKENTRY **lockentry)
{
	if(lockentry != NULL && *lockentry != NULL)
	{
		_http_allocator(_http_allocator_user_data, *lockentry, 0);
		*lockentry = NULL;
	}
}

int
dav_process_lockentry(DAV_LOCKENTRY *lockentry, XML_NODE *node)
{
	XML_NODE *node_cursor;
	for(node_cursor = node->first_child_node; node_cursor != NULL; node_cursor = node_cursor->next_node)
	{
		if(is_xml_node(node_cursor, "lockscope", "DAV:"))
		{
			lockentry->lockscope = get_lockscope(node_cursor->first_child_node);
		}
		else if(is_xml_node(node_cursor, "locktype", "DAV:"))
		{
			lockentry->locktype = get_locktype(node_cursor->first_child_node);
		}
	}
	return HT_OK;
}

void append_lockentry(DAV_SUPPORTEDLOCK *supportedlock, DAV_LOCKENTRY *lockentry)
{
	if(supportedlock->first_lockentry == NULL)
	{
		supportedlock->first_lockentry = lockentry;
	}
	else
	{
		supportedlock->last_lockentry->next_lockentry = lockentry;
	}
	lockentry->prev_lockentry = supportedlock->last_lockentry;
	supportedlock->last_lockentry = lockentry;
}

void
dav_supportedlock_destroy(DAV_SUPPORTEDLOCK **supportedlock)
{
	DAV_LOCKENTRY *lockentry_cursor = NULL, *next_lockentry = NULL;
	if(supportedlock != NULL && *supportedlock != NULL)
	{
		for(lockentry_cursor = (*supportedlock)->first_lockentry; lockentry_cursor != NULL; lockentry_cursor = next_lockentry)
		{
			next_lockentry = lockentry_cursor->next_lockentry;
			dav_lockentry_destroy(&lockentry_cursor);
		}
		_http_allocator(_http_allocator_user_data, *supportedlock, 0);
		*supportedlock = NULL;
	}
}

int
dav_process_supportedlock(DAV_SUPPORTEDLOCK *supportedlock, XML_NODE *node)
{
	XML_NODE *node_cursor;
	DAV_LOCKENTRY *new_lockentry;
	for(node_cursor = node->first_child_node; node_cursor != NULL; node_cursor = node_cursor->next_node)
	{
		if(is_xml_node(node_cursor, "lockentry", "DAV:"))
		{
			new_lockentry = (DAV_LOCKENTRY *) _http_allocator(_http_allocator_user_data, 0, sizeof(DAV_LOCKENTRY));
			if(new_lockentry == NULL)
			{
				return HT_MEMORY_ERROR;
			}
			memset(new_lockentry, 0, sizeof(DAV_SUPPORTEDLOCK));
			dav_process_lockentry(new_lockentry, node_cursor);
			append_lockentry(supportedlock, new_lockentry);
		}
	}
	return HT_OK;
}

int
get_resourcetype(XML_NODE *node)
{
	if(is_xml_node(node, "collection", "DAV:"))
	{
		return DAV_RESOURCETYPE_COLLECTION;
	}
	return DAV_RESOURCETYPE_OTHER;
}

void
append_external_prop(DAV_PROP *prop, XML_NODE *node)
{
	if(prop->first_external_prop == NULL)
	{
		prop->first_external_prop = node;
	}
	else
	{
		prop->last_external_prop->next_node = node;
	}
	node->prev_node = prop->last_external_prop;
	prop->last_external_prop = node;
}

void
dav_activelock_destroy(DAV_ACTIVELOCK **activelock)
{
	if(activelock != NULL && *activelock != NULL)
	{
		_http_allocator(_http_allocator_user_data, (*activelock)->owner, 0);
		_http_allocator(_http_allocator_user_data, (*activelock)->locktoken, 0);
		_http_allocator(_http_allocator_user_data, (*activelock)->timeout, 0);
		_http_allocator(_http_allocator_user_data, *activelock, 0);
		*activelock = NULL;
	}
}

int
dav_process_activelock(DAV_ACTIVELOCK *activelock, XML_NODE *node)
{
	XML_NODE *node_cursor;
	for(node_cursor = node->first_child_node; node_cursor != NULL; node_cursor = node_cursor->next_node)
	{
		if(is_xml_node(node_cursor, "lockscope", "DAV:"))
		{
			activelock->lockscope = get_lockscope(node_cursor->first_child_node);
		}
		else if(is_xml_node(node_cursor, "locktype", "DAV:"))
		{
			activelock->locktype = get_locktype(node_cursor->first_child_node);
		}
		else if(is_xml_node(node_cursor, "owner", "DAV:"))
		{
			if(node_cursor->first_child_node != NULL && node_cursor->first_child_node->data != NULL && activelock->owner == NULL)
			{
				activelock->owner = wd_strdup(node_cursor->first_child_node->data);
			}
		}
		else if(is_xml_node(node_cursor, "depth", "DAV:"))
		{
			if(node_cursor->data != NULL)
			{
				if(strcasecmp(node_cursor->data, "Infinity") == 0)
				{
					activelock->depth = HT_INFINITY;
				}
				else
				{
					activelock->depth = strtol(node_cursor->data, NULL, 10);
				}
			}
		}
		else if(is_xml_node(node_cursor, "timeout", "DAV:"))
		{
			if(node_cursor->data != NULL && activelock->timeout == NULL)
			{
				activelock->timeout = wd_strdup(node_cursor->data);
			}
		}
		else if(is_xml_node(node_cursor, "locktoken", "DAV:"))
		{
			if(node_cursor->first_child_node != NULL && is_xml_node(node_cursor->first_child_node, "href", "DAV:"))
			{
				if(node_cursor->first_child_node->data != NULL && activelock->locktoken == NULL)
				{
					activelock->locktoken = wd_strdup(node_cursor->first_child_node->data);
				}
			}
		}
	}
	return HT_OK;
}

void
dav_lockdiscovery_destroy(DAV_LOCKDISCOVERY **lockdiscovery)
{
	DAV_ACTIVELOCK *activelock_cursor = NULL, *next_activelock = NULL;
	if(lockdiscovery != NULL && *lockdiscovery != NULL)
	{
		for(activelock_cursor = (*lockdiscovery)->first_activelock; activelock_cursor != NULL; activelock_cursor = next_activelock)
		{
			next_activelock = activelock_cursor->next_activelock;
			dav_activelock_destroy(&activelock_cursor);
		}
		_http_allocator(_http_allocator_user_data, *lockdiscovery, 0);
		*lockdiscovery = NULL;
	}
}

void 
append_activelock(DAV_LOCKDISCOVERY *lockdiscovery, DAV_ACTIVELOCK *activelock)
{
	if(lockdiscovery->first_activelock == NULL)
	{
		lockdiscovery->first_activelock = activelock;
	}
	else
	{
		lockdiscovery->last_activelock->next_activelock = activelock;
	}
	activelock->prev_activelock = lockdiscovery->last_activelock;
	lockdiscovery->last_activelock = activelock;
}

int
dav_process_lockdiscovery(DAV_LOCKDISCOVERY *lockdiscovery, XML_NODE *node)
{
	XML_NODE *node_cursor;
	DAV_ACTIVELOCK *new_activelock;
	for(node_cursor = node->first_child_node; node_cursor != NULL; node_cursor = node_cursor->next_node)
	{
		if(is_xml_node(node_cursor, "activelock", "DAV:"))
		{
			new_activelock = (DAV_ACTIVELOCK *) _http_allocator(_http_allocator_user_data, 0, sizeof(DAV_ACTIVELOCK));
			if(new_activelock == NULL)
			{
				return HT_MEMORY_ERROR;
			}
			memset(new_activelock, 0, sizeof(DAV_ACTIVELOCK));
			dav_process_activelock(new_activelock, node_cursor);
			append_activelock(lockdiscovery, new_activelock);
		}
	}
	return HT_OK;
};

DAV_ACTIVELOCK *
dav_find_activelock(DAV_PROP *prop, DAV_LOCKSCOPE lockscope, DAV_LOCKTYPE locktype)
{
	DAV_ACTIVELOCK *activelock_cursor = NULL;
	if(prop == NULL || prop->lockdiscovery == NULL)
	{
		return NULL;
	}
	for(activelock_cursor = prop->lockdiscovery->first_activelock; activelock_cursor != NULL; activelock_cursor = activelock_cursor->next_activelock)
	{
		if(activelock_cursor->lockscope == lockscope && activelock_cursor->locktype == locktype)
		{
			return activelock_cursor;
		}
	}
	return NULL;
}

void
dav_prop_destroy(DAV_PROP **prop)
{
	XML_NODE *external_prop_cursor, *next_node;
	if(prop != NULL && *prop != NULL)
	{
		for(external_prop_cursor = (*prop)->first_external_prop; external_prop_cursor != NULL; external_prop_cursor = next_node)
		{
			next_node = external_prop_cursor->next_node;
			xml_node_destroy(&external_prop_cursor);
		}
		dav_lockdiscovery_destroy(&(*prop)->lockdiscovery);
		dav_supportedlock_destroy(&(*prop)->supportedlock);
		_http_allocator(_http_allocator_user_data, (*prop)->getcontenttype, 0);
		_http_allocator(_http_allocator_user_data, (*prop)->displayname, 0);
		_http_allocator(_http_allocator_user_data, (*prop)->status_msg, 0);
		_http_allocator(_http_allocator_user_data, *prop, 0);
		*prop = NULL;
	}
}

int
dav_process_prop(DAV_PROP *prop, XML_NODE *node)
{
	XML_NODE *node_cursor;
	XML_NODE *external_prop;
	for(node_cursor = node->first_child_node; node_cursor != NULL; node_cursor = node_cursor->next_node)
	{
		if(is_xml_node(node_cursor, "lockdiscovery", "DAV:"))
		{
			prop->lockdiscovery = (DAV_LOCKDISCOVERY *) _http_allocator(_http_allocator_user_data, 0, sizeof(DAV_LOCKDISCOVERY));
			if(prop->lockdiscovery == NULL)
			{
				return HT_MEMORY_ERROR;
			}
			memset(prop->lockdiscovery, 0, sizeof(DAV_LOCKDISCOVERY));
			dav_process_lockdiscovery(prop->lockdiscovery, node_cursor);
		}
		else if(is_xml_node(node_cursor, "creationdate", "DAV:"))
		{
			if(node_cursor->data != NULL && prop->creationdate == 0)
			{
				prop->creationdate = get_time_from_string(node_cursor->data);
			}
		}
		else if(is_xml_node(node_cursor, "getlastmodified", "DAV:"))
		{
			if(node_cursor->data != NULL && prop->getlastmodified == 0)
			{
				prop->getlastmodified = get_time_from_string(node_cursor->data);
			}
		}
		else if(is_xml_node(node_cursor, "getcontentlength", "DAV:"))
		{
			if(node_cursor->data != NULL && prop->getcontentlength == 0)
			{
				prop->getcontentlength = strtol(node_cursor->data, NULL, 10);
			}
		}
		else if(is_xml_node(node_cursor, "displayname", "DAV:"))
		{
			if(node_cursor->data != NULL && prop->displayname == NULL)
			{
				prop->displayname = wd_strdup(node_cursor->data);
				if(prop->displayname == NULL)
				{
					return HT_MEMORY_ERROR;
				}
			}
		}
		else if(is_xml_node(node_cursor, "getcontenttype", "DAV:"))
		{
			if(node_cursor->data != NULL && prop->displayname == NULL)
			{
				prop->getcontenttype = wd_strdup(node_cursor->data);
				if(prop->getcontenttype == NULL)
				{
					return HT_MEMORY_ERROR;
				}
			}
		}
		else if(is_xml_node(node_cursor, "resourcetype", "DAV:"))
		{
			if(node_cursor->first_child_node != NULL && prop->resourcetype == 0)
			{
				prop->resourcetype = get_resourcetype(node_cursor->first_child_node);
			}
		}
		else if(is_xml_node(node_cursor, "supportedlock", "DAV:"))
		{
			prop->supportedlock = (DAV_SUPPORTEDLOCK *) _http_allocator(_http_allocator_user_data, 0, sizeof(DAV_SUPPORTEDLOCK));
			if(prop->supportedlock == NULL)
			{
				return HT_MEMORY_ERROR;
			}
			memset(prop->supportedlock, 0, sizeof(DAV_SUPPORTEDLOCK));
			dav_process_supportedlock(prop->supportedlock, node_cursor);
		}
		else
		{
			if(xml_node_duplicate(node_cursor, &external_prop) == XT_OK)
			{
				append_external_prop(prop, external_prop);
			}
		}
	}
	return HT_OK;
}

void
dav_propstat_destroy(DAV_PROPSTAT **propstat)
{
	if(propstat != NULL && *propstat != NULL)
	{
		dav_prop_destroy(&(*propstat)->prop);
		_http_allocator(_http_allocator_user_data, (*propstat)->status_msg, 0);
		_http_allocator(_http_allocator_user_data, *propstat, 0);
		*propstat = NULL;
	}
}

int
dav_process_propstat(DAV_PROPSTAT *propstat, XML_NODE *node)
{
	XML_NODE *node_cursor;
	const char *space1, *space2;
	for(node_cursor = node->first_child_node; node_cursor != NULL; node_cursor = node_cursor->next_node)
	{
		if(is_xml_node(node_cursor, "status", "DAV:"))
		{
			if(node_cursor->data != NULL && propstat->status_code == 0 && propstat->status_msg == NULL)
			{
				space1 = strchr(node_cursor->data, ' ');
				if(space1 != NULL && strlen(space1) >= 4)
				{
					propstat->status_code = (space1[1] - '0') * 100 + (space1[2] - '0') * 10 + (space1[3] - '0');
					space2 = strchr(space1 + 1, ' ');
					if(space2 != NULL) 
					{
						propstat->status_msg = wd_strdup(space2 + 1);
					}
				}
			}
		}
		else if(is_xml_node(node_cursor, "prop", "DAV:"))
		{
			propstat->prop = (DAV_PROP *) _http_allocator(_http_allocator_user_data, 0, sizeof(DAV_PROP));
			if(propstat->prop == NULL)
			{
				return HT_MEMORY_ERROR;
			}
			memset(propstat->prop, 0, sizeof(DAV_PROP));
			dav_process_prop(propstat->prop, node_cursor);
		}
	}
	return HT_OK;
}

void
dav_response_destroy(DAV_RESPONSE **response)
{
	DAV_PROPSTAT *propstat_cursor = NULL, *next_propstat = NULL;
	if(response != NULL && *response != NULL)
	{
		for(propstat_cursor = (*response)->first_propstat; propstat_cursor != NULL; propstat_cursor = next_propstat)
		{
			next_propstat = propstat_cursor->next_propstat;
			dav_propstat_destroy(&propstat_cursor);
		}
		_http_allocator(_http_allocator_user_data, (*response)->href, 0);
		_http_allocator(_http_allocator_user_data, *response, 0);
		*response = NULL;
	}
}

void 
append_propstat(DAV_RESPONSE *response, DAV_PROPSTAT *propstat)
{
	if(response->first_propstat == NULL)
	{
		response->first_propstat = propstat;
	}
	else
	{
		response->last_propstat->next_propstat = propstat;
	}
	propstat->prev_propstat = response->last_propstat;
	response->last_propstat = propstat;
}

int
dav_process_response(DAV_RESPONSE *response, XML_NODE *node)
{
	XML_NODE *node_cursor;
	DAV_PROPSTAT *new_propstat;
	for(node_cursor = node->first_child_node; node_cursor != NULL; node_cursor = node_cursor->next_node)
	{
		if(is_xml_node(node_cursor, "href", "DAV:"))
		{
			if(node_cursor->data != NULL && response->href == NULL)
			{
				response->href = wd_strdup_url_decoded(node_cursor->data);
				if(response->href == NULL)
				{
					return HT_MEMORY_ERROR;
				}
			}
		}
		else if(is_xml_node(node_cursor, "propstat", "DAV:"))
		{
			new_propstat = (DAV_PROPSTAT *) _http_allocator(_http_allocator_user_data, 0, sizeof(DAV_PROPSTAT));
			if(new_propstat == NULL)
			{
				return HT_MEMORY_ERROR;
			}
			memset(new_propstat, 0, sizeof(DAV_PROPSTAT));
			dav_process_propstat(new_propstat, node_cursor);
			append_propstat(response, new_propstat);
		}
	}
	return HT_OK;
}

DAV_PROP *
dav_find_prop(DAV_RESPONSE *response, int status_code_range_start, int status_code_range_end)
{
	DAV_PROPSTAT *propstat_cursor = NULL;
	if(response == NULL)
	{
		return NULL;
	}
	for(propstat_cursor = response->first_propstat; propstat_cursor != NULL; propstat_cursor = propstat_cursor->next_propstat)
	{
		if(propstat_cursor->status_code >= status_code_range_start && propstat_cursor->status_code <= status_code_range_end)
		{
			return propstat_cursor->prop;
		}
	}
	return NULL;
}

void
dav_multistatus_destroy(DAV_MULTISTATUS **multistatus)
{
	DAV_RESPONSE *response_cursor = NULL, *next_response = NULL;
	if(multistatus != NULL && *multistatus != NULL)
	{
		for(response_cursor = (*multistatus)->first_response; response_cursor != NULL; response_cursor = next_response)
		{
			next_response = response_cursor->next_response;
			dav_response_destroy(&response_cursor);
		}
		_http_allocator(_http_allocator_user_data, *multistatus, 0);
		*multistatus = NULL;
	}
}

void append_response(DAV_MULTISTATUS *multistatus, DAV_RESPONSE *response)
{
	if(multistatus->first_response == NULL)
	{
		multistatus->first_response = response;
	}
	else
	{
		multistatus->last_response->next_response = response;
	}
	response->prev_response = multistatus->last_response;
	multistatus->last_response = response;
}

int
dav_process_multistatus(DAV_MULTISTATUS *multistatus, XML_NODE *node)
{
	XML_NODE *node_cursor;
	DAV_RESPONSE *new_response;
	for(node_cursor = node->first_child_node; node_cursor != NULL; node_cursor = node_cursor->next_node)
	{
		if(is_xml_node(node_cursor, "response", "DAV:"))
		{
			new_response = (DAV_RESPONSE *) _http_allocator(_http_allocator_user_data, 0, sizeof(DAV_RESPONSE));
			if(new_response == NULL)
			{
				return HT_MEMORY_ERROR;
			}
			memset(new_response, 0, sizeof(DAV_RESPONSE));
			dav_process_response(new_response, node_cursor);
			append_response(multistatus, new_response);
		}
	}
	return HT_OK;
}

int
dav_create_multistatus_from_storage(DAV_MULTISTATUS **multistatus, HTTP_STORAGE *storage)
{
	XML_TREE *tree = NULL;
	DAV_MULTISTATUS *new_multistatus = NULL;
	int error = HT_OK;
	new_multistatus = (DAV_MULTISTATUS *) _http_allocator(_http_allocator_user_data, 0, sizeof(DAV_MULTISTATUS));
	if(new_multistatus == NULL)
	{
		return HT_MEMORY_ERROR;
	}
	memset(new_multistatus, 0, sizeof(DAV_MULTISTATUS));
	if((error = xml_tree_build_from_storage(&tree, storage)) != HT_OK)
	{
		_http_allocator(_http_allocator_user_data, new_multistatus, 0);
		return error;
	}
	if(tree->root_node->first_child_node != NULL)
	{
		dav_process_multistatus(new_multistatus, tree->root_node->first_child_node);
	}
	xml_tree_destroy(&tree);
	*multistatus  = new_multistatus;
	return HT_OK;
}

void
dav_propfind_destroy(DAV_PROPFIND **propfind)
{
	if(propfind != NULL && *propfind != NULL)
	{
		dav_prop_destroy(&(*propfind)->prop);
		_http_allocator(_http_allocator_user_data, *propfind, 0);
		*propfind = NULL;
	}
}

int
dav_create_propfind(DAV_PROPFIND **propfind)
{
	DAV_PROPFIND *new_propfind = NULL;
	if(propfind == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	new_propfind = (DAV_PROPFIND *) _http_allocator(_http_allocator_user_data, 0, sizeof(DAV_PROPFIND));
	if(new_propfind == NULL)
	{
		return HT_MEMORY_ERROR;
	}
	memset(new_propfind, 0, sizeof(DAV_PROPFIND));
	*propfind = new_propfind;
	return HT_OK;
}

int
dav_add_find_custom_prop(DAV_PROPFIND *propfind, const char *name, const char *ns)
{
	XML_NODE *external_prop;
	int error;
	if(propfind == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	if(propfind->prop == NULL)
	{
		propfind->prop = _http_allocator(_http_allocator_user_data, 0, sizeof(DAV_PROP));
		if(propfind->prop == NULL)
		{
			return HT_MEMORY_ERROR;
		}
		memset(propfind->prop, 0, sizeof(DAV_PROP));
	}
	if((error = xml_node_create(&external_prop, name, ns, NULL)) != XT_OK)
	{
		return error;
	}
	append_external_prop(propfind->prop, external_prop);
	return HT_OK;
}

int
dav_add_find_prop(DAV_PROPFIND *propfind, const char *name)
{
	return dav_add_find_custom_prop(propfind, name, "DAV:");
}

int
dav_add_find_prop_comma_delimited(DAV_PROPFIND *propfind, const char *additional_prop)
{
	char *name = NULL, *ns = NULL;
	int name_start_index = 0, ns_start_index = 0, name_length, ns_length;
	size_t i;
	int error;
	if(additional_prop == NULL)
	{
		return HT_OK;
	}
	for(i = 0; i <= strlen(additional_prop); i++)
	{
		if(additional_prop[i] == ',' || additional_prop[i] == '\0')
		{
			ns_length = name_start_index - ns_start_index - 1;
			name_length = i - name_start_index;
			if(ns_length > 0)
			{
				ns = wd_strndup(additional_prop + ns_start_index, ns_length);
				name = wd_strndup(additional_prop + name_start_index, name_length);
				if((error = dav_add_find_custom_prop(propfind, name, ns)) != HT_OK)
				{
					_http_allocator(_http_allocator_user_data, ns, 0);
					_http_allocator(_http_allocator_user_data, name, 0);
					return error;
				}
				_http_allocator(_http_allocator_user_data, ns, 0);
				_http_allocator(_http_allocator_user_data, name, 0);
			}
			else
			{
				name = wd_strndup(additional_prop + name_start_index, name_length);
				if((error = dav_add_find_prop(propfind, name)) != HT_OK)
				{
					_http_allocator(_http_allocator_user_data, name, 0);
					return error;   
				}
				_http_allocator(_http_allocator_user_data, name, 0);
			}
			name_start_index = ns_start_index = i + 1;
		}
		else if(additional_prop[i] == ':')
		{
			name_start_index = i + 1;
		}
	}
	return HT_OK;
}

int
dav_set_find_all_prop(DAV_PROPFIND *propfind)
{
	if(propfind == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	propfind->allprop = HT_TRUE;
	return HT_OK;
}

int
dav_write_propfind_to_storage(DAV_PROPFIND *propfind, HTTP_STORAGE *storage)
{
	XML_TREE *tree = NULL;
	XML_NODE *node_cursor = NULL;
	int error = HT_OK;
	if((error = xml_tree_create(&tree)) != HT_OK)
	{
		return error;
	}
	if((error = xml_tree_start_node(tree, "propfind", "DAV:")) != HT_OK)
	{
		xml_tree_destroy(&tree);
		return error;
	}
	if(propfind->prop != NULL)
	{
		if((error = xml_tree_start_node(tree, "prop", "DAV:")) != HT_OK)
		{
		}
		for(node_cursor = propfind->prop->first_external_prop; node_cursor != NULL; node_cursor = node_cursor->next_node)
		{
			if((error = xml_tree_add_node(tree, node_cursor->name, node_cursor->ns, NULL)) != HT_OK)
			{
				xml_tree_destroy(&tree);
				return error;
			}
		}
		if((error = xml_tree_close_node(tree, "prop", "DAV:")) != HT_OK)
		{
			xml_tree_destroy(&tree);
			return error;
		}
	}
	else if(propfind->allprop)
	{
		if((error = xml_tree_add_node(tree, "allprop", "DAV:", NULL)) != HT_OK)
		{
			xml_tree_destroy(&tree);
			return error;
		}
	}
	else if(propfind->propname)
	{
		if((error = xml_tree_add_node(tree, "propname", "DAV:", NULL)) != HT_OK)
		{
			xml_tree_destroy(&tree);
			return error;
		}
	}
	if((error = xml_tree_close_node(tree, "propfind", "DAV:")) != HT_OK)
	{
		xml_tree_destroy(&tree);
		return error;
	}
	if((error = xml_tree_write_to_storage(tree, storage)) != HT_OK)
	{
		xml_tree_destroy(&tree);
		return error;
	}
	xml_tree_destroy(&tree);
	return HT_OK;
}

void
dav_lockinfo_destroy(DAV_LOCKINFO **lockinfo)
{
	if(lockinfo != NULL && *lockinfo != NULL)
	{
		_http_allocator(_http_allocator_user_data, (*lockinfo)->owner, 0);
		_http_allocator(_http_allocator_user_data, *lockinfo, 0);
		*lockinfo = NULL;
	}
}

int
dav_create_lockinfo(DAV_LOCKINFO **lockinfo, DAV_LOCKSCOPE lockscope, DAV_LOCKTYPE locktype, const char *owner)
{
	DAV_LOCKINFO *new_lockinfo = NULL;
	if(lockinfo == NULL)
	{
		return HT_INVALID_ARGUMENT;
	}
	new_lockinfo = (DAV_LOCKINFO *) _http_allocator(_http_allocator_user_data, 0, sizeof(DAV_LOCKINFO));
	if(new_lockinfo == NULL)
	{
		return HT_MEMORY_ERROR;
	}
	memset(new_lockinfo, 0, sizeof(DAV_LOCKINFO));
	new_lockinfo->lockscope = lockscope;
	new_lockinfo->locktype = locktype;
	new_lockinfo->owner = wd_strdup(owner);
	if(new_lockinfo->owner == NULL)
	{
		dav_lockinfo_destroy(&new_lockinfo);
		return HT_MEMORY_ERROR;
	}
	*lockinfo = new_lockinfo;
	return HT_OK;
}

int
dav_write_lockinfo_to_storage(DAV_LOCKINFO *lockinfo, HTTP_STORAGE *storage)
{
	XML_TREE *tree = NULL;
	XML_NODE *node_cursor = NULL;
	int error = HT_OK;
	if((error = xml_tree_create(&tree)) != HT_OK)
	{
		return error;
	}
	if((error = xml_tree_start_node(tree, "lockinfo", "DAV:")) != HT_OK)
	{
		xml_tree_destroy(&tree);
		return error;
	}
	if((error = xml_tree_start_node(tree, "lockscope", "DAV:")) != HT_OK
			|| (error = xml_tree_add_node(tree, lockscope_to_str(lockinfo->lockscope), "DAV:", NULL)) != HT_OK
			|| (error = xml_tree_close_node(tree, "lockscope", "DAV:")) != HT_OK)
	{
		xml_tree_destroy(&tree);
		return error;
	}
	if((error = xml_tree_start_node(tree, "locktype", "DAV:")) != HT_OK
			|| (error = xml_tree_add_node(tree, locktype_to_str(lockinfo->locktype), "DAV:", NULL)) != HT_OK
			|| (error = xml_tree_close_node(tree, "locktype", "DAV:")) != HT_OK)
	{
		xml_tree_destroy(&tree);
		return error;
	}
	if(lockinfo->owner != NULL)
	{
		if((error = xml_tree_start_node(tree, "owner", "DAV:")) != HT_OK
				|| (error = xml_tree_add_node(tree, "href", "DAV:", lockinfo->owner)) != HT_OK
				|| (error = xml_tree_close_node(tree, "owner", "DAV:")) != HT_OK)
		{
			xml_tree_destroy(&tree);
			return error;
		}
	}
	if((error = xml_tree_close_node(tree, "lockinfo", "DAV:")) != HT_OK)
	{
		xml_tree_destroy(&tree);
		return error;
	}
	if((error = xml_tree_write_to_storage(tree, storage)) != HT_OK)
	{
		xml_tree_destroy(&tree);
		return error;
	}
	xml_tree_destroy(&tree);
	return HT_OK;
}

int
dav_create_prop_from_storage(DAV_PROP **prop, HTTP_STORAGE *storage)
{
	XML_TREE *tree = NULL;
	DAV_PROP *new_prop = NULL;
	int error = HT_OK;
	new_prop = (DAV_PROP *) _http_allocator(_http_allocator_user_data, 0, sizeof(DAV_PROP));
	if(new_prop == NULL)
	{
		return HT_MEMORY_ERROR;
	}
	memset(new_prop, 0, sizeof(DAV_PROP));
	if((error = xml_tree_build_from_storage(&tree, storage)) != HT_OK)
	{
		_http_allocator(_http_allocator_user_data, new_prop, 0);
		return error;
	}
	if(tree->root_node->first_child_node != NULL)
	{
		dav_process_prop(new_prop, tree->root_node->first_child_node);
	}
	xml_tree_destroy(&tree);
	*prop  = new_prop;
	return HT_OK;
}

