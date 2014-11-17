#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "treecli_parser.h"


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


int32_t treecli_parser_pos_print(const struct treecli_parser_pos *pos) {
	assert(pos != NULL);
	
	printf("/");
	for (uint32_t i = 0; i < pos->depth; i++) {
		if (i > 0) {
			printf("/");
		}
		if (pos->levels[i].node != NULL) {
			printf("%s", pos->levels[i].node->name);
			continue;
		}
		if (pos->levels[i].dnode != NULL) {
			printf("<dnode>");
			continue;
		}
		/* shouldn't go here, either node or dnode must not be NULL */
		assert(0);
	}

	return TREECLI_PRINT_POS_OK;
}


/**
 * Search command line and try to get next token. Token is a word describing one
 * subnode, command or value name consisting of alphanumeric characters (lower and
 * uppoer case), underscore, dash, dot and slash.
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
	       (**pos >= '0' && **pos <= '9') || **pos == '-' || **pos == '_' || **pos == '.' || **pos == '/') {
	
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
	treecli_parser_pos_init(&(parser->pos));
	
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
 * commands (if requested).
 * 
 * @param parser A parser context used to do command parsing.
 * @param line String with node names, commands and value set/get specifications.
 * 
 * @return TREECLI_PARSER_PARSE_LINE_OK if the whole line was parsed successfully.
 */
int32_t treecli_parser_parse_line(struct treecli_parser *parser, const char *line) {

	int32_t res;
	char *pos = (char *)line;
	char *token = NULL;
	uint32_t len;

	/* save current position in case we will need to rollback the whole command */
	struct treecli_parser_pos parser_pos_saved;
	treecli_parser_pos_copy(&parser_pos_saved, &(parser->pos));
	
	/* Last matched token type - set to 1 if it was subnode or dnode. */
	int last_match_subnode = 0;

	while ((res = treecli_token_get(parser, &pos, &token, &len)) == TREECLI_TOKEN_GET_OK) {

		last_match_subnode = 0;
		struct treecli_matches matches;

		int32_t ret = treecli_parser_get_matches(parser, token, len, &matches);
		parser->error_pos = (uint32_t)(pos - line) - len;

		if (ret == TREECLI_PARSER_GET_MATCHES_FAILED) {
			treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
			return TREECLI_PARSER_PARSE_LINE_FAILED;
		}

		if (ret == TREECLI_PARSER_GET_MATCHES_NONE) {
			treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
			return TREECLI_PARSER_PARSE_LINE_NO_MATCHES;
		}
		if (ret == TREECLI_PARSER_GET_MATCHES_MULTIPLE) {
			treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
			return TREECLI_PARSER_PARSE_LINE_MULTIPLE_MATCHES;
		}

		if (matches.count == 1) {

			if (ret == TREECLI_PARSER_GET_MATCHES_TOP) {
				if (treecli_parser_pos_root(&(parser->pos)) != TREECLI_PARSER_POS_ROOT_OK) {
					treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
					return TREECLI_PARSER_PARSE_LINE_CANNOT_MOVE;
				}
				last_match_subnode = 1;
				printf("going to top\n");
			}

			if (ret == TREECLI_PARSER_GET_MATCHES_UP) {
				if (treecli_parser_pos_up(&(parser->pos)) != TREECLI_PARSER_POS_UP_OK) {
					treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
					return TREECLI_PARSER_PARSE_LINE_CANNOT_MOVE;
				}
				last_match_subnode = 1;
				printf("going up\n");
			}

			if (ret == TREECLI_PARSER_GET_MATCHES_SUBNODE) {
				if (treecli_parser_pos_move(&(parser->pos), &(struct treecli_parser_pos_level){.node = matches.subnode, .dnode = NULL}) != TREECLI_PARSER_POS_MOVE_OK) {
					treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
					return TREECLI_PARSER_PARSE_LINE_CANNOT_MOVE;
				}
				last_match_subnode = 1;
				printf("going to subnode %s\n", matches.subnode->name);
			}

			if (ret == TREECLI_PARSER_GET_MATCHES_COMMAND) {
				printf("command %s matched\n", matches.command->name);
				if (parser->allow_exec) {
					if (matches.command->exec != NULL) {
						matches.command->exec(parser, matches.command->exec_context);
					}
				}
			}

			if (ret == TREECLI_PARSER_GET_MATCHES_VALUE) {
				printf("value %s matched\n", matches.value->name);
				/* TODO: get operation */
				/* TODO: get literal */
			}
		}
	}

	/* This is the single point of return if no error occurs. Determine if the
	 * last matched token was a subnode or dynamically generated subnode. In thet
	 * case keep current parsing position as is (we intentionally issued a
	 * command to change parsing position). Otherwise reset parsing position
	 * back. */
	
	if (last_match_subnode == 0) {
		printf("resetting parsing position\n");
		treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
	}
	return TREECLI_PARSER_PARSE_LINE_OK;
}


int32_t treecli_parser_set_print_handler(struct treecli_parser *parser, int32_t (*print_handler)(const char *line, void *ctx), void *ctx) {
	assert(parser != NULL);
	assert(print_handler != NULL);

	parser->print_handler = print_handler;
	parser->print_handler_ctx = ctx;

	return TREECLI_PARSER_SET_PRINT_HANDLER_OK;
	
}


int32_t treecli_parser_get_matches(struct treecli_parser *parser, char *token, uint32_t len, struct treecli_matches *matches) {
	assert(parser != NULL);
	assert(token != NULL);
	assert(matches != NULL);

	/* We are trying to get matches of different types. Following variables will
	 * be modified if a match occurs. We need to initialize them first. */
	matches->count = 0;
	int32_t ret = TREECLI_PARSER_GET_MATCHES_NONE;

	/* Get current working position - we are matching only at this level. */
	struct treecli_node node;
	int32_t res = treecli_parser_get_current_node(&(parser->pos), &node);
	if (res == TREECLI_PARSER_GET_CURRENT_NODE_ROOT) {
		memcpy(&node, parser->top, sizeof(struct treecli_node));
	} else if (res == TREECLI_PARSER_GET_CURRENT_NODE_FAILED) {
		return TREECLI_PARSER_GET_MATCHES_FAILED;
	}

	/* Try to match common tokens - one level up (..) and go to root (/) */
	if (!strncmp(token, "..", 2) && len == 2) {
		matches->count++;
		ret = TREECLI_PARSER_GET_MATCHES_UP;
	}
	if (!strncmp(token, "/", 1) && len == 1) {
		matches->count++;
		ret = TREECLI_PARSER_GET_MATCHES_TOP;
	}

	/* Match all statically set subnodes. */
	const struct treecli_node *n = node.subnodes;
	while (n != NULL) {
		if (len <= strlen(n->name) && !strncmp(token, n->name, len)) {
			matches->count++;
			matches->subnode = n;
			ret = TREECLI_PARSER_GET_MATCHES_SUBNODE;
		}
		n = n->next;
	}

	/* TODO: get all dynamic node constructors and iterate over them with
	 * valid index ranges. Then construct nodes and try to match them as usual. */

	/* Match values at current position/level. */
	const struct treecli_value *v = node.values;
	while (v != NULL) {
		if (len <= strlen(v->name) && !strncmp(token, v->name, len)) {
			matches->count++;
			matches->value = v;
			ret = TREECLI_PARSER_GET_MATCHES_VALUE;
		}
		v = v->next;
	}

	/* And match commands at current position/level. */
	const struct treecli_command *c = node.commands;
	while (c != NULL) {
		if (len <= strlen(c->name) && !strncmp(token, c->name, len)) {
			matches->count++;
			matches->command = c;
			ret = TREECLI_PARSER_GET_MATCHES_COMMAND;
		}
		c = c->next;
	}

	if (matches->count == 1) {
		assert(ret != TREECLI_PARSER_GET_MATCHES_NONE);
		return ret;
	} else if (matches->count > 1) {
		return TREECLI_PARSER_GET_MATCHES_MULTIPLE;
	}

	return TREECLI_PARSER_GET_MATCHES_NONE;
}


int32_t treecli_parser_pos_move(struct treecli_parser_pos *pos, struct treecli_parser_pos_level *level) {
	assert(pos != NULL);
	assert(level != NULL);
	
	if (pos->depth < TREECLI_TREE_MAX_DEPTH) {
		memcpy(&(pos->levels[pos->depth]), level, sizeof(struct treecli_parser_pos_level));
		pos->depth++;
		
		return TREECLI_PARSER_POS_MOVE_OK;
	}

	return TREECLI_PARSER_POS_MOVE_FAILED;
}


int32_t treecli_parser_pos_up(struct treecli_parser_pos *pos) {
	assert(pos != NULL);
	
	if (pos->depth > 0) {
		pos->depth--;
		return TREECLI_PARSER_POS_UP_OK;
	}
	return TREECLI_PARSER_POS_UP_FAILED;
}


int32_t treecli_parser_pos_root(struct treecli_parser_pos *pos) {
	assert(pos != NULL);
	
	pos->depth = 0;
	
	return TREECLI_PARSER_POS_ROOT_OK;
}


int32_t treecli_parser_pos_copy(struct treecli_parser_pos *pos, struct treecli_parser_pos *src) {
	assert(pos != NULL);
	assert(src != NULL);
	
	memcpy(pos, src, sizeof(struct treecli_parser_pos));
	
	return TREECLI_PARSER_POS_COPY_OK;
}


int32_t treecli_parser_pos_init(struct treecli_parser_pos *pos) {
	assert(pos != NULL);
	
	pos->depth = 0;
	
	return TREECLI_PARSER_POS_INIT_OK;
}


int32_t treecli_parser_get_current_node(struct treecli_parser_pos *pos, struct treecli_node *node) {
	assert(pos != NULL);
	assert(node != NULL);

	if (pos->depth == 0) {
		return TREECLI_PARSER_GET_CURRENT_NODE_ROOT;
	} else {
		if (pos->levels[pos->depth - 1].node != NULL) {
			memcpy(node, pos->levels[pos->depth - 1].node, sizeof(struct treecli_node));
			return TREECLI_PARSER_GET_CURRENT_NODE_OK;
		} else if (pos->levels[pos->depth - 1].dnode != NULL) {
			return TREECLI_PARSER_GET_CURRENT_NODE_FAILED;
		} else {
			return TREECLI_PARSER_GET_CURRENT_NODE_FAILED;
		}
	}
	
	return TREECLI_PARSER_GET_CURRENT_NODE_FAILED;
}


