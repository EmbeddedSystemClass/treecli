#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "treecli.h"


int32_t treecli_print_tree(const struct treecli_node *top, int32_t indent) {
	assert(top != NULL);
	
	const struct treecli_node *n = top;
	while (n != NULL) {
		for (int32_t i = 0; i < indent; i++) {
			printf(" ");
		}
		printf("* %s\n", n->name);
		if (n->subnodes) {
			treecli_print_tree(n->subnodes, indent + 4);
		}
		
		n = n->next;
	}
	
	return TREECLI_PRINT_TREE_OK;
}


/**
 * Search command line and try to get next token. Token is a word describing one
 * subnode, command or value name consisting of alphanumeric characters (lower and
 * uppoer case), underscore and dash.
 * Input line position is being incremented during search and after execution it
 * points to a position where the search can continue (this apply also if function
 * fails).
 * 
 * @param parser TODO: remove
 * @param pos Pointer to initial string position where the search should begin.
 * @param token Pointer which will point to start of the token after successful
 *              execution.
 * @param len Length of the token.
 * 
 * @return TREECLI_TOKEN_GET_OK if valid token was found or
 *         TREECLI_TOKEN_GET_UNEXPECTED if invalid characters were found or
 *         TREECLI_TOKEN_GET_NONE if end of line was reached or
 *         TREECLI_TOKEN_GET_FAILED otherwise.
 */
int32_t treecli_token_get(struct treecli_parser *parser, char **pos, char **token, uint32_t *len) {

	/* eat all whitespaces */
	while (**pos == ' ' || **pos == '\t') {
		(*pos)++;
	}

	/* we have reached end of line */
	if (**pos == 0) {
		return TREECLI_TOKEN_GET_NONE;
	}

	/* mark start of the token and go forward while valid characters are found*/
	*token = *pos;
	while ((**pos >= 'a' && **pos <= 'z') || (**pos >= 'A' && **pos <= 'Z') ||
	       (**pos >= '0' && **pos <= '9') || **pos == '-' || **pos == '_') {
	
		(*pos)++;
	}
	*len = *pos - *token;

	/* no valid token has been found */
	if (*len == 0) {
		return TREECLI_TOKEN_GET_UNEXPECTED;
	}
	
	return TREECLI_TOKEN_GET_OK;
}



/**
 * Initializes parser context. It is used to parse lines of configuration, execute
 * commands and manipulate configuration variables. Parser operates on a
 * configuration tree defined by its top level node.
 * 
 * @param parser A treecli parser context to initialize.
 * @param top Top node of configuration structure used during parsing.
 * 
 * @return TREECLI_PARSER_INIT_OK on success or
 *         TREECLI_PARSER_INIT_FAILED otherwise.
 */
int32_t treecli_parser_init(struct treecli_parser *parser, const struct treecli_node *top) {
	assert(parser != NULL);
	assert(top != NULL);
	
	parser->top = top;
	
	return TREECLI_PARSER_INIT_OK;
}


/**
 * Frees previously created parser context. Nothing to do here yet, placeholder.
 * 
 * @param parser A parser context to free.
 * 
 * @return TREECLI_PARSER_FREE_OK.
 */
int32_t treecli_parser_free(struct treecli_parser *parser) {
	assert(parser != NULL);
	
	return TREECLI_PARSER_FREE_OK;
}


/**
 * Function parses one line of commands and performs configuration tree traversal
 * to set active node for command execution, sets or reads values and executes
 * commands.
 * 
 * @param parser A parser context used to do command parsing.
 * @param line String with node names, commands and value set/get specifications.
 * 
 * @return TREECLI_PARSER_PARSE_LINE_OK if the whole line was parsed successfully.
 */
int32_t treecli_parser_parse_line(struct treecli_parser *parser, const char *line) {
	
}


int32_t treecli_parser_set_print_handler(struct treecli_parser *parser, int32_t (*print_handler)(char *line, void *ctx), void *ctx) {
	assert(parser != NULL);
	assert(print_handler != NULL);
	
	parser->print_handler = print_handler;
	parser->print_handler_ctx = ctx;
	
	return TREECLI_PARSER_SET_PRINT_HANDLER_OK;
	
}


int32_t treecli_parse_token(struct treecli_parser *parser, const char *token) {
	/* ako parameter berie aktualny node, na ktorom zacina parsovanie.
	 * dopredu sa moze hybat len rekurziou, dozadu sa moze hybat volne.
	 * ".." posuva o poziciu spat, "/" posuva na zaciatocnu poziciu (to je taka,
	 * kde je parent NULL)
	 * 
	 * parsuje tak, ze najskor zozerie vsetky whitespaces z aktualneho stringu
	 * (ten je tiez ako parameter), potom ziska dalsi token, potom ziska matches
	 * pre dany token (subnods, commands, values osobitne). Ak je sucet matches
	 * presne 1, moze ist dalej. Ak je sucet 0, to znamena, ze je to nejaka
	 * blbost a vrati chybu (neparsovatelny vstup). Ak je matches viac, vrati
	 * ich pocet (resp. navratovu hodnotu multiple matches).
	 * Ak je taka poziadavka, pri parsovani vola callback funkciu, ktora ma
	 * ako parameter postupne vsetky matches pre dany node.
	 * Ak je to poziadavka, pri parsovani tiez volat setter/getter funkcie
	 * (pre hodnoty) alebo exec funkcie (pre prikazy)
	 */
	
}


int32_t treecli_matches() {
	/* hlada vsetky matches pre specifikovany node (staticke ak dynamicke)
	 */
	
}
