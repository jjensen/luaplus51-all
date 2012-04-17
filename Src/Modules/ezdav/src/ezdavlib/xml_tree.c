#include <string.h>
#include <stdlib.h>
#include "strutl.h"
#include "xml_tree.h"

extern http_allocator _http_allocator;
extern void* _http_allocator_user_data;

void 
append_child_xml_node(XML_NODE *parent_node, XML_NODE *new_node)
{
	if(parent_node->first_child_node == NULL)
	{
		parent_node->first_child_node = new_node;
	}
	else
	{
		parent_node->last_child_node->next_node = new_node;
	}
	new_node->parent_node = parent_node;
	new_node->prev_node = parent_node->last_child_node;
	parent_node->last_child_node = new_node;
}

void
xml_node_destroy(XML_NODE **node)
{
	XML_NODE *node_cursor = NULL, *next_node = NULL;
	if(node != NULL && *node != NULL)
	{
		for(node_cursor = (*node)->first_child_node; node_cursor != NULL; node_cursor = next_node)
		{
			next_node = node_cursor->next_node;
			xml_node_destroy(&node_cursor);
		}
		_http_allocator(_http_allocator_user_data, (*node)->name, 0);
		_http_allocator(_http_allocator_user_data, (*node)->ns, 0);
		_http_allocator(_http_allocator_user_data, (*node)->data, 0);
		_http_allocator(_http_allocator_user_data, *node, 0);
		*node = NULL;
	}
}

int
xml_node_create(XML_NODE **node, const char *name, const char *ns, const char *data)
{
	XML_NODE *new_node = NULL;
	int name_length = 0;
	if(node == NULL || name == NULL)
	{
		return XT_INVALID_ARGUMENT;
	}
	new_node = (XML_NODE *) _http_allocator(_http_allocator_user_data, 0, sizeof(XML_NODE));
	if(new_node == NULL)
	{
		return XT_MEMORY_ERROR;
	}
	memset(new_node, 0, sizeof(XML_NODE));
	new_node->name = wd_strdup(name);
	if(ns != NULL)
	{
		new_node->ns = wd_strdup(ns);
	}
	if(data != NULL)
	{
		new_node->data = wd_strdup(data);
	}
	if(new_node->name == NULL || (ns != NULL && new_node->ns == NULL) || (data != NULL && new_node->data == NULL))
	{
		xml_node_destroy(&new_node);
		return XT_MEMORY_ERROR;
	}
	name_length = strlen(new_node->name);
	if(name_length > 0 && new_node->name[name_length - 1] == '/')
	{
		new_node->name[name_length - 1] = '\0';
	}
	*node = new_node;
	return XT_OK;
}

void
xml_tree_destroy(XML_TREE **tree)
{
	if(tree != NULL && *tree != NULL)
	{
		xml_node_destroy(&(*tree)->root_node);
		_http_allocator(_http_allocator_user_data, *tree, 0);
	}
}

int 
xml_tree_create(XML_TREE **tree)
{
	XML_TREE *new_tree;
	XML_NODE *new_root_node;
	new_tree = (XML_TREE *) _http_allocator(_http_allocator_user_data, 0, sizeof(XML_TREE));
	new_root_node = (XML_NODE *) _http_allocator(_http_allocator_user_data, 0, sizeof(XML_NODE));
	if(new_tree == NULL || new_root_node == NULL)
	{
		xml_tree_destroy(&new_tree);
		return XT_MEMORY_ERROR;
	}
	memset(new_tree, 0, sizeof(XML_TREE));
	memset(new_root_node, 0, sizeof(XML_NODE));
	new_tree->root_node = new_root_node;
	new_tree->current_node = new_tree->root_node;
	*tree = new_tree;
	return XT_OK;
}

void xml_tree_start_element_handler(void *userData, const XML_Char *name, const XML_Char **atts);
void xml_tree_end_element_handler(void *userData, const XML_Char *name);
void xml_tree_character_data_handler(void *userData,	const XML_Char *s, int len);

int
xml_tree_start_node(XML_TREE *tree, const char *name, const char *ns)
{
	XML_NODE *new_node;
	int error = XT_OK;
	if((error = xml_node_create(&new_node, name, ns, NULL)) != XT_OK)
	{
		return error;
	}
	append_child_xml_node(tree->current_node, new_node);
	if(name[0] != '\0' && name[strlen(name) - 1] != '/')
	{
		tree->current_node = new_node;
	}
	return XT_OK;
}

int
xml_tree_add_data(XML_TREE *tree, const char *data, int len)
{
	char *new_text;
	int old_len;
	if(tree->current_node->data == NULL)
	{
		new_text = (char *) _http_allocator(_http_allocator_user_data, 0, len + 1);
		if(new_text == NULL)
		{
			return XT_MEMORY_ERROR;
		}
		memcpy(new_text, data, len);
		new_text[len] = '\0';
	}
	else
	{
		old_len = strlen(tree->current_node->data);
		new_text = (char *) _http_allocator(_http_allocator_user_data, tree->current_node->data, old_len + len + 1);
		if(new_text == NULL)
		{
			return XT_MEMORY_ERROR;
		}
		memcpy(new_text + old_len, data, len);
		new_text[old_len + len] = '\0';
	}
	tree->current_node->data = new_text;
	return XT_OK;
}

int
xml_tree_close_node(XML_TREE *tree, const char *name, const char *ns)
{
	tree->current_node = tree->current_node->parent_node;
	return XT_OK;
}

int
xml_tree_add_node(XML_TREE *tree, const char *name, const char *ns, const char *data)
{
	char *new_name = NULL;
	if(data == NULL)
	{
		new_name = (char *) _http_allocator(_http_allocator_user_data, 0, strlen(name) + 2);
		if(new_name == NULL)
		{
			return XT_MEMORY_ERROR;
		}
		strcpy(new_name, name);
		strcat(new_name, "/");
		xml_tree_start_node(tree, new_name, ns);
		_http_allocator(_http_allocator_user_data, new_name, 0);
	}
	else
	{
		xml_tree_start_node(tree, name, ns);
		xml_tree_add_data(tree, data, strlen(data));
		xml_tree_close_node(tree, name, ns);
	}
	return XT_OK;
}

int
xml_node_duplicate(XML_NODE *src_node, XML_NODE **dest_node)
{
	XML_NODE *node_cursor = NULL, *new_node = NULL, *new_child_node;
	int error = XT_OK;
	if(src_node == NULL || dest_node == NULL)
	{
		return XT_INVALID_ARGUMENT;
	}
	if((error = xml_node_create(&new_node, src_node->name, src_node->ns, src_node->data)) != XT_OK)
	{
		return error;
	}
	for(node_cursor = src_node->first_child_node; node_cursor != NULL; node_cursor = node_cursor->next_node)
	{
		if((error = xml_node_duplicate(node_cursor, &new_child_node)) != XT_OK)
		{
			xml_node_destroy(&new_node);
			return error;
		}
		append_child_xml_node(new_node, new_child_node);
	}
	*dest_node = new_node;
	return XT_OK;
}

void 
xml_tree_start_element_handler(void *userData, const XML_Char *name, const XML_Char **atts)
{
	char *new_ns = NULL;
	const char *new_name = name, *slash;
	int ns_len;
	slash = strrchr(name, '/');
	if(slash != NULL)
	{
		new_name = slash + 1;
		ns_len = strlen(name) - strlen(slash);
		new_ns = (char *) _http_allocator(_http_allocator_user_data, 0, ns_len + 1);
		if(new_ns != NULL)
		{
			memcpy(new_ns, name, ns_len);
			new_ns[ns_len] = '\0';
		}
	}
	xml_tree_start_node((XML_TREE *) userData, new_name, new_ns);
	_http_allocator(_http_allocator_user_data, new_ns, 0);
}

void 
xml_tree_end_element_handler(void *userData, const XML_Char *name)
{
	char *new_ns = NULL;
	const char *new_name = name, *slash;
	int ns_len;
	slash = strrchr(name, '/');
	if(slash != NULL)
	{
		new_name = slash + 1;
		ns_len = strlen(name) - strlen(slash);
		new_ns = (char *) _http_allocator(_http_allocator_user_data, 0, ns_len + 1);
		if(new_ns != NULL)
		{
			memcpy(new_ns, name, ns_len);
			new_ns[ns_len] = '\0';
		}
	}
	xml_tree_close_node((XML_TREE *) userData, new_name, new_ns);
	_http_allocator(_http_allocator_user_data, new_ns, 0);
}

void 
xml_tree_character_data_handler(void *userData,	const XML_Char *s, int len)
{
	xml_tree_add_data((XML_TREE *) userData, s, len);
}

static void *xml_tree_expat_malloc(size_t size)
{
	return _http_allocator(_http_allocator_user_data, 0, size);
}

static void *xml_tree_expat_realloc(void *ptr, size_t size)
{
	return _http_allocator(_http_allocator_user_data, ptr, size);
}

static void xml_tree_expat_free(void *ptr)
{
	_http_allocator(_http_allocator_user_data, ptr, 0);
}

int 
xml_tree_build_from_storage(XML_TREE **tree, HTTP_STORAGE *storage)
{
	XML_TREE *new_tree;
	XML_Parser parser;
	char read_buffer[128];
	int read_count;
	XML_Char tmp[2];
	XML_Memory_Handling_Suite expatMemoryHandlingSuite;
	if(tree == NULL || storage == NULL)
	{
		return XT_INVALID_ARGUMENT;
	}
	if(xml_tree_create(&new_tree) != XT_OK)
	{
		return XT_MEMORY_ERROR;
	}

	expatMemoryHandlingSuite.malloc_fcn = xml_tree_expat_malloc;
	expatMemoryHandlingSuite.realloc_fcn = xml_tree_expat_realloc;
	expatMemoryHandlingSuite.free_fcn = xml_tree_expat_free;
	*tmp = '/';
	parser = XML_ParserCreate_MM(NULL, &expatMemoryHandlingSuite, tmp);
	if(parser == NULL)
	{
		return XT_FATAL_ERROR;
	}
	XML_SetUserData(parser, (void *) new_tree);
	XML_SetElementHandler(parser, xml_tree_start_element_handler, xml_tree_end_element_handler);
	XML_SetCharacterDataHandler(parser, xml_tree_character_data_handler);
	http_storage_seek(storage, 0);
	while(http_storage_read(storage, read_buffer, 128, &read_count) == HT_OK && read_count > 0)
	{
		if(XML_Parse(parser, read_buffer, read_count, read_count != 128) == 0)
		{
			return XT_FATAL_ERROR;
		}
	}
	XML_ParserFree(parser);
	*tree = new_tree;
	return XT_OK;
}

typedef struct xml_ns_token_table XML_NS_TOKEN_TABLE;

struct xml_ns_token_table {
	const XML_Char *ns[26];
	int count;
};

void
init_token_table(XML_NS_TOKEN_TABLE *token_table, XML_NS_TOKEN_TABLE *parent_token_table)
{
	if(parent_token_table == NULL)
	{
		memset(token_table, 0, sizeof(XML_NS_TOKEN_TABLE));
	}
	else
	{
		memcpy(token_table, parent_token_table, sizeof(XML_NS_TOKEN_TABLE));
	}
}

int
get_ns_token(const XML_Char *ns, XML_NS_TOKEN_TABLE *token_table)
{
	int i;
	for(i = 0; i < token_table->count; i++)
	{
		if(strcmp(token_table->ns[i], ns) == 0)
		{
			return i;
		}
	}
	return -1;
}

int
add_ns_token(const XML_Char *ns, XML_NS_TOKEN_TABLE *token_table)
{
	int i;
	if(token_table->count < 26)
	{
		i = token_table->count++;
		token_table->ns[i] = ns;
		return i;
	}
	return -1;
}

int
add_children_ns_token(XML_NODE *node, XML_NS_TOKEN_TABLE *token_table)
{
	XML_NS_TOKEN_TABLE test_token_table;
	XML_NODE *node_cursor;
	int token, token_use_count[26], new_token_count = 0, i;
	init_token_table(&test_token_table, NULL);
	memset(token_use_count, 0, sizeof(int) * 26);
	for(node_cursor = node->first_child_node; node_cursor != NULL; node_cursor = node_cursor->next_node)
	{
		if(get_ns_token(node->ns, token_table) == -1)
		{
			if((token = get_ns_token(node_cursor->ns, &test_token_table)) != -1 || (token = add_ns_token(node_cursor->ns, &test_token_table)) != -1)
			{
				token_use_count[token]++;
			}
		}
	}
	for(i = 0; i < 26; i++)
	{
		if(token_use_count[i] > 1)
		{
			add_ns_token(test_token_table.ns[i], token_table);
			new_token_count++;
		}
	}
	return new_token_count;
}

int
xml_tree_write_node_to_storage(XML_NODE *node, XML_NS_TOKEN_TABLE *parent_token_table, HTTP_STORAGE *storage)
{
	XML_NS_TOKEN_TABLE local_token_table;
	XML_NODE *node_cursor;
	int token = -1, i;
	char buffer[4];
	init_token_table(&local_token_table, parent_token_table);
	if(node->ns != NULL)
	{
		if((token = get_ns_token(node->ns, &local_token_table)) == -1)
		{
			token = add_ns_token(node->ns, &local_token_table);
		}
	}
	http_storage_write(storage, "<", 1);
	if(token != -1)
	{
		buffer[0] = token + 'A';
		buffer[1] = ':';
		http_storage_write(storage, buffer, 2);
	}
	http_storage_write(storage, node->name, strlen(node->name));
	add_children_ns_token(node, &local_token_table);
	for(i = parent_token_table != NULL ? parent_token_table->count : 0; i < local_token_table.count; i++)
	{
		http_storage_write(storage, " xmlns:", 7);
		buffer[0] = i + 'A';
		buffer[1] = '=';
		buffer[2] = '"';
		http_storage_write(storage, buffer, 3);
		http_storage_write(storage, local_token_table.ns[i], strlen(local_token_table.ns[i]));
		http_storage_write(storage, "\"", 1);
	}
	if(node->data == NULL && node->first_child_node == NULL)
	{
		http_storage_write(storage, "/>\n", 3);
	}
	else
	{
		http_storage_write(storage, ">", 1);
		if(node->data != NULL)
		{
			http_storage_write(storage, node->data, strlen(node->data));
		}
		else
		{
			http_storage_write(storage, "\n", 1);
		}
		for(node_cursor = node->first_child_node; node_cursor != NULL; node_cursor = node_cursor->next_node)
		{
			xml_tree_write_node_to_storage(node_cursor, &local_token_table, storage);
		}
		http_storage_write(storage, "</", 2);
		if(token != -1)
		{
			buffer[0] = token + 'A';
			buffer[1] = ':';
			http_storage_write(storage, buffer, 2);
		}
		http_storage_write(storage, node->name, strlen(node->name));
		http_storage_write(storage, ">\n", 2);
	}
	return XT_OK;
}

int
xml_tree_write_to_storage(XML_TREE *tree, HTTP_STORAGE *storage)
{
	XML_NODE *node_cursor;
	const char *header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	http_storage_write(storage, header, strlen(header));
	for(node_cursor = tree->root_node->first_child_node; node_cursor != NULL; node_cursor = node_cursor->next_node)
	{
		xml_tree_write_node_to_storage(node_cursor, NULL, storage);
	}
	return XT_OK;
}
