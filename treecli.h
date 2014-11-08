

#ifndef _TREECLI_H_
#define _TREECLI_H_



#define TREECLI_VALUE_UINT32 0

/**
 * Dnode specifies how are node childs generated during runtime. It can be used
 * to dynamically generate configuration subtrees for components not known at
 * compile time (for example - network interfaces).
 */
struct treecli_dnode {
	
	const struct treecli_dnode *next;
};

struct treecli_command {
	
	/* TODO:
	 * function pointer to execute the command
	 */
	const char *name;
	const struct treecli_command *next;
};

/**
 * Single node in the tree hierarchy. Nodes can be chained together on the same
 * level using "next" field (creating linked list of nodes, last one must have it
 * set to NULL). There must be only one top level node (conventionally named "/")
 * which is used to reference the whole tree.
 */
struct treecli_value {
	const char *name;

	/* Pointer to variable with actual value. Optional, could be ommited
	 * (value getter/setter can be used instead). If neither one is specified,
	 * value always reads as (uint32)0, write has ho effect */
	void *value;
	
	/* Value assigned after initialization. It has no effect if value getter
	 * is valid. */
	void *default_value;
	
	/* Type of value or value getter/setter */
	int value_type;
	
	/* TODO: value getter/setter functions */
	/* TODO: value boundaries */
	/* TODO: valid values enum/iterator */

	const struct treecli_value *next;
};

struct treecli_node {
	/* TODO:
	 */
	
	const char *name;
	
	const struct treecli_node *subnodes;
	const struct treecli_dnode *dsubnodes;
	const struct treecli_command *commands;
	const struct treecli_value *values;
	
	const struct treecli_node *next;
};





struct treecli_parser {
	const struct treecli_node *top;
	
	int32_t (*print_handler)(char *line, void *ctx);
	void *print_handler_ctx;
	
	/* TODO: output function */
	
};


struct treecli_context {
	
	
	
};







int32_t treecli_print_tree(const struct treecli_node *top, int32_t indent);
#define TREECLI_PRINT_TREE_OK 0
#define TREECLI_PRINT_TREE_FAILED -1

int32_t treecli_token_get(struct treecli_parser *parser, char **pos, char **token, uint32_t *len);
#define TREECLI_TOKEN_GET_OK 0
#define TREECLI_TOKEN_GET_FAILED -1
#define TREECLI_TOKEN_GET_NONE -2
#define TREECLI_TOKEN_GET_UNEXPECTED -3


int32_t treecli_parser_init(struct treecli_parser *parser, const struct treecli_node *top);
#define TREECLI_PARSER_INIT_OK 0
#define TREECLI_PARSER_INIT_FAILED -1

int32_t treecli_parser_free(struct treecli_parser *parser);
#define TREECLI_PARSER_FREE_OK 0
#define TREECLI_PARSER_FREE_FAILED -1

int32_t treecli_parser_parse_line(struct treecli_parser *parser, const char *line);
#define TREECLI_PARSER_PARSE_LINE_OK 0
#define TREECLI_PARSER_PARSE_LINE_FAILED -1

int32_t treecli_parser_set_print_handler(struct treecli_parser *parser, int32_t (*print_handler)(char *line, void *ctx), void *ctx);
#define TREECLI_PARSER_SET_PRINT_HANDLER_OK 0
#define TREECLI_PARSER_SET_PRINT_HANDLER_FAILED -1




#endif
