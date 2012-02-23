#ifndef __XML_TREE_H__
#define __XML_TREE_H__
#include <expat.h>
#include "http.h"

#define XT_FATAL_ERROR			0xFFFF
#define XT_MEMORY_ERROR			0xFFFE
#define XT_ILLEGAL_OPERATION	0xFFFD
#define XT_INVALID_ARGUMENT		0xFFFC
#define XT_OK					0

typedef struct xml_node XML_NODE;
typedef struct xml_tree XML_TREE;

struct xml_tree {
	XML_Parser parser;
	XML_NODE *root_node;
	XML_NODE *current_node;
};

struct xml_node {
	XML_Char *name;
	XML_Char *data;
	XML_Char *ns;
	XML_NODE *parent_node;
	XML_NODE *prev_node;
	XML_NODE *next_node;
	XML_NODE *first_child_node;
	XML_NODE *last_child_node;
};


int xml_node_create(XML_NODE **node, const char *name, const char *ns, const char *data);
void xml_node_destroy(XML_NODE **node);
int xml_node_duplicate(XML_NODE *src_node, XML_NODE **dest_node);
int xml_tree_create(XML_TREE **tree);
void xml_tree_destroy(XML_TREE **tree);
int xml_tree_build_from_storage(XML_TREE **tree, HTTP_STORAGE *storage);
int xml_tree_start_node(XML_TREE *tree, const char *name, const char *ns);
int xml_tree_add_data(XML_TREE *tree, const char *data, int len);
int xml_tree_close_node(XML_TREE *tree, const char *name, const char *ns);
int xml_tree_add_node(XML_TREE *tree, const char *name, const char *ns, const char *data);
int xml_tree_write_to_storage(XML_TREE *tree, HTTP_STORAGE *storage);

#endif
