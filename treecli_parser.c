/**
 * Copyright (c) 2014, Marek Koza (qyx@krtko.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "treecli_parser.h"


int __attribute__((weak)) u_assert_func(const char *a, const char *f, int n) {
	printf("Assertion '%s' failed in %s, line %d\n", a, f, n);
	abort();
	return 1;
}

/*
int32_t treecli_print_tree(const struct treecli_node *top, int32_t indent) {
	if (u_assert(top != NULL)) {
		return TREECLI_PRINT_TREE_FAILED;
	}

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
*/

int32_t treecli_parser_pos_print(struct treecli_parser *parser, bool no_delimiter) {
	if (u_assert(parser != NULL) ||
	    u_assert(parser->print_handler != NULL)) {
		return TREECLI_PARSER_POS_PRINT_FAILED;
	}

	uint32_t len = 0;
	if (no_delimiter == false) {
		parser->print_handler("/", parser->print_handler_ctx);
		len += 1;
	}

	for (uint32_t i = 0; i < parser->pos.depth; i++) {
		if (i > 0) {
			if (no_delimiter) {
				parser->print_handler(" ", parser->print_handler_ctx);
			} else {
				parser->print_handler("/", parser->print_handler_ctx);
			}
			len += 1;
		}
		if (parser->pos.levels[i].node != NULL) {
			parser->print_handler(parser->pos.levels[i].node->name, parser->print_handler_ctx);
			len += strlen(parser->pos.levels[i].node->name);
			continue;
		}
		if (parser->pos.levels[i].dnode != NULL) {
			/* construct dynamic node */
			struct treecli_node dnode;
			memset(&dnode, 0, sizeof(dnode));
			char dnode_name[100];
			dnode.name = dnode_name;

			if (parser->pos.levels[i].dnode->create != NULL &&
			    parser->pos.levels[i].dnode->create(parser, parser->pos.levels[i].dnode_index, &dnode, parser->pos.levels[i].dnode->create_context) >= 0) {
				parser->print_handler(dnode.name, parser->print_handler_ctx);
				len += strlen(dnode.name);
			} else {
				parser->print_handler("<?>", parser->print_handler_ctx);
				len += 3;
			}

			continue;
		}
		/* shouldn't go here, either node or dnode must not be NULL */
		u_assert(0);
	}

	return len;
}


int32_t treecli_token_get(struct treecli_parser *parser, const char **pos, const char **token, uint32_t *len) {
	if (u_assert(parser != NULL) ||
	    u_assert(pos != NULL) ||
	    u_assert(token != NULL) ||
	    u_assert(len != NULL)) {
		return TREECLI_TOKEN_GET_FAILED;
	}

	/* eat all whitespaces */
	while (**pos == ' ' || **pos == '\t') {
		(*pos)++;
	}

	/* we have reached end of line */
	if (**pos == 0) {
		return TREECLI_TOKEN_GET_NONE;
	}

	/* mark start of the token */
	*token = *pos;
	if (**pos == '(' || **pos == ')' || **pos == '=' || **pos == '/' || **pos == '?') {
		/* Single character tokens. */
		(*pos)++;
	} else if ((**pos >= '0' && **pos <= '9') || **pos == '-' || **pos == '.') {
		/* Numbers. */
		(*pos)++;
		while ((**pos >= '0' && **pos <= '9') || **pos == '.') {
			(*pos)++;
		}
	} else if ((**pos >= 'a' && **pos <= 'z') || (**pos >= 'A' && **pos <= 'Z') || **pos == '_') {
		/* Alphanumeric tokens. */
		(*pos)++;
		while ((**pos >= 'a' && **pos <= 'z') || (**pos >= 'A' && **pos <= 'Z') || (**pos >= '0' && **pos <= '9') || **pos == '_' || **pos == '-' || **pos == ':') {
			(*pos)++;
		}
	} else if (**pos == '.') {
		/* Double dot. */
		(*pos)++;
		while (**pos == '.') {
			(*pos)++;
		}
	} else if (**pos == '"') {
		(*pos)++;
		while (**pos != '"') {
			if (**pos != '\0') {
				(*pos)++;
			} else {
				return TREECLI_TOKEN_GET_FAILED;
			}
		}
		(*pos)++;

	}

	*len = *pos - *token;

	/* no valid token has been found */
	if (*len == 0) {
		return TREECLI_TOKEN_GET_UNEXPECTED;
	}

	return TREECLI_TOKEN_GET_OK;
}


int32_t treecli_parser_init(struct treecli_parser *parser, const struct treecli_node *top) {
	if (u_assert(parser != NULL) ||
	    u_assert(top != NULL)) {
		return TREECLI_PARSER_INIT_FAILED;
	}

	memset(parser, 0, sizeof(struct treecli_parser));

	parser->top = top;
	if (treecli_parser_pos_init(&(parser->pos)) != TREECLI_PARSER_POS_INIT_OK) {
		return TREECLI_PARSER_INIT_FAILED;

	}
	treecli_parser_set_mode(parser, TREECLI_PARSER_DEFAULT);

	return TREECLI_PARSER_INIT_OK;
}


int32_t treecli_parser_free(struct treecli_parser *parser) {
	if (u_assert(parser != NULL)) {
		return TREECLI_PARSER_FREE_FAILED;
	}

	return TREECLI_PARSER_FREE_OK;
}


int32_t treecli_parser_parse_line(struct treecli_parser *parser, const char *line) {
	if (u_assert(parser != NULL) ||
	    u_assert(line != NULL)) {
		return TREECLI_PARSER_PARSE_LINE_FAILED;
	}

	int32_t res;
	const char *pos = line;
	const char *token = NULL;
	uint32_t len;

	/* save current position in case we will need to rollback the whole command */
	struct treecli_parser_pos parser_pos_saved;
	treecli_parser_pos_copy(&parser_pos_saved, &(parser->pos));

	/* We need an information if the last matched action was a tree traversal
	 * action or not. We are setting this to 1 when we move in the tree. */
	int last_match_subnode = 0;

	parser->parsing_context = TREECLI_PARSER_CONTEXT_NODE;

	/* Iterate over the whole command and get all tokens */
	while ((res = treecli_token_get(parser, &pos, &token, &len)) == TREECLI_TOKEN_GET_OK) {

		last_match_subnode = 0;
		struct treecli_matches matches;

		/* these status variables are used to determine position of last
		 * matched token (whether it was successful or not). Can be used
		 * by callers when error occurs. */
		parser->error_pos = (uint32_t)(pos - line) - len;
		parser->error_len = len;

		int32_t ret = treecli_parser_get_matches(parser, token, len, &matches);

		/* We requested matches for a token we got previously. Now lets
		 * handle all uncommon states (failed, no matches, multiple matches).
		 * In all these cases we need to go back to position at which
		 * we started parsing. */
		if (ret == TREECLI_PARSER_GET_MATCHES_FAILED) {
			treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
			return TREECLI_PARSER_PARSE_LINE_FAILED;
		}

		if (ret == TREECLI_PARSER_GET_MATCHES_NONE) {
			treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
			return TREECLI_PARSER_PARSE_LINE_NO_MATCHES;
		}
		if (ret == TREECLI_PARSER_GET_MATCHES_MULTIPLE) {
			/* This case is slightly different. We need to know the best
			 * match that occured for the purpose of possible autocompletion. */
			if ((parser->mode & TREECLI_PARSER_ALLOW_BEST_MATCH) && parser->best_match_handler) {
				parser->best_match_handler(matches.best_match, matches.best_match_len, parser->error_pos, parser->error_len, parser->best_match_handler_ctx);
			}
			treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
			return TREECLI_PARSER_PARSE_LINE_MULTIPLE_MATCHES;
		}

		/* Now handle all "normal" states - only one match occured. We are
		 * not returning in these cases. */
		if (matches.count == 1) {

			/* One single match is also the one that is the best. Call
			 * best match handler if requested. */
			if ((parser->mode & TREECLI_PARSER_ALLOW_BEST_MATCH) && parser->best_match_handler) {
				parser->best_match_handler(matches.best_match, matches.best_match_len, parser->error_pos, parser->error_len, parser->best_match_handler_ctx);
			}

			/* Following tokens make tree traversal actions - going to
			 * top of the tree, going one position up (to parent node),
			 * going to statically defined node or dynamically constructed
			 * node. These are the cases where we set last_match_subnode
			 * to 1 to indicate that tree traversal action occured. */
			if (ret == TREECLI_PARSER_GET_MATCHES_TOP) {
				if (treecli_parser_pos_root(&(parser->pos)) != TREECLI_PARSER_POS_ROOT_OK) {
					treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
					return TREECLI_PARSER_PARSE_LINE_CANNOT_MOVE;
				}
				last_match_subnode = 1;
			}

			if (ret == TREECLI_PARSER_GET_MATCHES_UP) {
				if (treecli_parser_pos_up(&(parser->pos)) != TREECLI_PARSER_POS_UP_OK) {
					treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
					return TREECLI_PARSER_PARSE_LINE_CANNOT_MOVE;
				}
				last_match_subnode = 1;
			}

			if (ret == TREECLI_PARSER_GET_MATCHES_SUBNODE) {
				if (treecli_parser_pos_move(&(parser->pos), &(struct treecli_parser_pos_level){.node = matches.subnode, .dnode = NULL}) != TREECLI_PARSER_POS_MOVE_OK) {
					treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
					return TREECLI_PARSER_PARSE_LINE_CANNOT_MOVE;
				}
				last_match_subnode = 1;
			}

			if (ret == TREECLI_PARSER_GET_MATCHES_DSUBNODE) {
				if (treecli_parser_pos_move(&(parser->pos), &(struct treecli_parser_pos_level){.node = NULL, .dnode = matches.dsubnode, .dnode_index = matches.dsubnode_index}) != TREECLI_PARSER_POS_MOVE_OK) {
					treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
					return TREECLI_PARSER_PARSE_LINE_CANNOT_MOVE;
				}
				last_match_subnode = 1;
			}

			/* If the current token is a command, we simply check if
			 * command exec callback is defined and try to execute it.
			 * Return value is also checked and need to be nonnegative.
			 * Otherwise we assume that command execution failed. */
			if (ret == TREECLI_PARSER_GET_MATCHES_COMMAND) {
				if ((parser->mode & TREECLI_PARSER_ALLOW_EXEC) && matches.command->exec != NULL) {
					if (matches.command->exec(parser, matches.command->exec_context) < 0) {
						treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
						return TREECLI_PARSER_PARSE_LINE_COMMAND_FAILED;
					};
				}
			}

			if (ret == TREECLI_PARSER_GET_MATCHES_VALUE) {
				if (parser->mode & TREECLI_PARSER_ALLOW_EXEC) {
					parser->parsing_value = matches.value;
				}
			}

			if (ret == TREECLI_PARSER_GET_MATCHES_VALUE_OPERATOR) {
				/** @todo Always assignment for now. Add code for multiple operators. */
				if (parser->mode & TREECLI_PARSER_ALLOW_EXEC) {
				}
			}

			if (ret == TREECLI_PARSER_GET_MATCHES_VALUE_LITERAL) {
				if (parser->mode & TREECLI_PARSER_ALLOW_EXEC) {
					if (treecli_parser_str_to_value(parser, parser->parsing_value, token, len) != TREECLI_PARSER_STR_TO_VALUE_OK) {
						treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
						return TREECLI_PARSER_PARSE_LINE_VALUE_FAILED;
					}
				}
			}

			if (ret == TREECLI_PARSER_GET_MATCHES_HELP) {
				if (parser->mode & TREECLI_PARSER_ALLOW_EXEC) {
					treecli_parser_help(parser);
				}
			}
		}
	}

	/* We can get here if everything went okay (we executed all actions specified
	 * on the command line, optionally we just autocompleted the last token.
	 * If the user wants to see some more commands to chain, give some more
	 * suggestions. */
	if (parser->mode & TREECLI_PARSER_ALLOW_SUGGESTIONS) {
		treecli_parser_set_mode(parser, TREECLI_PARSER_ALLOW_MATCHES);
		struct treecli_matches matches;
		treecli_parser_get_matches(parser, "", 0, &matches);
	}

	/* Reset the position if command execution was disabled or if the last
	 * executed action was not tree traversal */
	if (last_match_subnode == 0 || !(parser->mode & TREECLI_PARSER_ALLOW_EXEC)) {
		treecli_parser_pos_copy(&(parser->pos), &parser_pos_saved);
	}

	/* Handle some special cases, eg. no value literal at the end. Save position
	 * of the error. */
	parser->error_pos = (uint32_t)(pos - line) - len;
	parser->error_len = 0;

	if (parser->parsing_context == TREECLI_PARSER_CONTEXT_VALUE_LITERAL && (parser->mode & TREECLI_PARSER_ALLOW_EXEC)) {
		parser->error_pos += 2;
		return TREECLI_PARSER_PARSE_LINE_EXPECTING_VALUE;
	}

	if (res == TREECLI_TOKEN_GET_UNEXPECTED) {
		return TREECLI_PARSER_PARSE_LINE_UNEXPECTED_TOKEN;
	}

	if (res == TREECLI_TOKEN_GET_MALFORMED) {
		return TREECLI_PARSER_PARSE_LINE_MALFORMED_TOKEN;
	}

	return TREECLI_PARSER_PARSE_LINE_OK;
}


int32_t treecli_parser_set_print_handler(struct treecli_parser *parser, int32_t (*print_handler)(const char *line, void *ctx), void *ctx) {
	if (u_assert(parser != NULL) ||
	    u_assert(print_handler != NULL)) {
		return TREECLI_PARSER_SET_PRINT_HANDLER_FAILED;
	}

	parser->print_handler = print_handler;
	parser->print_handler_ctx = ctx;

	return TREECLI_PARSER_SET_PRINT_HANDLER_OK;
}


int32_t treecli_parser_set_match_handler(struct treecli_parser *parser, int32_t (*match_handler)(const char *token, enum treecli_match_type match_type, void *ctx), void *ctx) {
	if (u_assert(parser != NULL) ||
	    u_assert(match_handler != NULL)) {
		return TREECLI_PARSER_SET_MATCH_HANDLER_FAILED;
	}

	parser->match_handler = match_handler;
	parser->match_handler_ctx = ctx;

	return TREECLI_PARSER_SET_MATCH_HANDLER_OK;
}


int32_t treecli_parser_set_best_match_handler(struct treecli_parser *parser, int32_t (*best_match_handler)(const char *token, uint32_t token_len, uint32_t match_pos, uint32_t match_len, void *ctx), void *ctx) {
	if (u_assert(parser != NULL) ||
	    u_assert(best_match_handler != NULL)) {
		return TREECLI_PARSER_SET_BEST_MATCH_HANDLER_FAILED;
	}

	parser->best_match_handler = best_match_handler;
	parser->best_match_handler_ctx = ctx;

	return TREECLI_PARSER_SET_BEST_MATCH_HANDLER_OK;
}


uint32_t treecli_parser_strmatch(const char *s1, const char *s2) {
	if(u_assert(s1 != NULL) ||
	   u_assert(s2 != NULL)) {
		return 0;
	}

	int32_t p = 0;
	while (*s1 && *s2 && *s1 == *s2) {
		s1++;
		s2++;
		p++;
	}

	return p;
}


int32_t treecli_parser_resolve_match(struct treecli_parser *parser, struct treecli_matches *matches, const char *token) {
	if(u_assert(parser != NULL) ||
	   u_assert(token != NULL) ||
	   u_assert(matches != NULL)) {
		return TREECLI_PARSER_RESOLVE_MATCH_FAILED;
	}

	if (parser->mode & TREECLI_PARSER_ALLOW_MATCHES) {

		/** @todo determine the match type */
		if (parser->match_handler) {
			parser->match_handler(token, TREECLI_MATCH_TYPE_NODE, parser->match_handler_ctx);
		}
	}

	/* Find out if the current match is better than the previously saved
	 * best match. If yes, save it. */
	if (matches->count <= 1) {
		strcpy(matches->best_match, token);
		matches->best_match_len = strlen(token);
	} else {
		uint32_t r = treecli_parser_strmatch(matches->best_match, token);
		if (r < matches->best_match_len) {
			matches->best_match_len = r;
		}
	}

	return TREECLI_PARSER_RESOLVE_MATCH_OK;
}


int32_t treecli_parser_try_match(struct treecli_parser *parser, struct treecli_matches *matches, const char *token, uint32_t len, const char *str) {
	if (u_assert(parser != NULL) ||
	    u_assert(matches != NULL) ||
	    u_assert(token != NULL) ||
	    u_assert(str != NULL)) {
		return TREECLI_PARSER_TRY_MATCH_FAILED;
	}

	if (!strncmp(token, str, len) && len <= strlen(str)) {
		matches->count++;
		treecli_parser_resolve_match(parser, matches, str);

		return TREECLI_PARSER_TRY_MATCH_OK;
	}

	return TREECLI_PARSER_TRY_MATCH_FAILED;
}


int32_t treecli_parser_get_matches(struct treecli_parser *parser, const char *token, uint32_t len, struct treecli_matches *matches) {
	if (u_assert(parser != NULL) ||
	    u_assert(token != NULL) ||
	    u_assert(matches != NULL)) {
		return TREECLI_PARSER_GET_MATCHES_FAILED;
	}

	/* We are trying to get matches of different types. Match count will
	 * be modified if a match occurs. */
	matches->count = 0;

	int32_t ret = TREECLI_PARSER_GET_MATCHES_NONE;

	/* Get current working position - we are matching only at this level. */
	struct treecli_node node;
	int32_t res = treecli_parser_get_current_node(parser, &node);
	if (res == TREECLI_PARSER_GET_CURRENT_NODE_ROOT) {
		memcpy(&node, parser->top, sizeof(struct treecli_node));
	} else if (res == TREECLI_PARSER_GET_CURRENT_NODE_FAILED) {
		return TREECLI_PARSER_GET_MATCHES_FAILED;
	}

	if (parser->parsing_context == TREECLI_PARSER_CONTEXT_VALUE_LITERAL) {
		/** @todo add matches of numbers, enums, etc. */
		ret = TREECLI_PARSER_GET_MATCHES_VALUE_LITERAL;
		parser->parsing_context = TREECLI_PARSER_CONTEXT_NODE;
		matches->count++;

	} else if (parser->parsing_context == TREECLI_PARSER_CONTEXT_NODE || parser->parsing_context == TREECLI_PARSER_CONTEXT_VALUE_OPERATOR) {

		/* Try to match special tokens */
		if (treecli_parser_try_match(parser, matches, token, len, "..") == TREECLI_PARSER_TRY_MATCH_OK) {
			ret = TREECLI_PARSER_GET_MATCHES_UP;
		}
		if (treecli_parser_try_match(parser, matches, token, len, "/") == TREECLI_PARSER_TRY_MATCH_OK) {
			ret = TREECLI_PARSER_GET_MATCHES_TOP;
		}
		if (treecli_parser_try_match(parser, matches, token, len, "?") == TREECLI_PARSER_TRY_MATCH_OK) {
			ret = TREECLI_PARSER_GET_MATCHES_HELP;
		}

		/* Match value operators. */
		if (parser->parsing_context == TREECLI_PARSER_CONTEXT_VALUE_OPERATOR &&
		    treecli_parser_try_match(parser, matches, token, len, "=") == TREECLI_PARSER_TRY_MATCH_OK) {
			ret = TREECLI_PARSER_GET_MATCHES_VALUE_OPERATOR;
			parser->parsing_context = TREECLI_PARSER_CONTEXT_VALUE_LITERAL;
		}

		/* Match all statically set subnodes. */
		if (node.subnodes != NULL) {
			const struct treecli_node *n;
			for (size_t i = 0; (n = (*(node.subnodes))[i]) != NULL; i++) {
				if (treecli_parser_try_match(parser, matches, token, len, n->name) == TREECLI_PARSER_TRY_MATCH_OK) {
					matches->subnode = n;
					ret = TREECLI_PARSER_GET_MATCHES_SUBNODE;
				}
			}
		}

		/* Match all dynamically constructed subnodes */
		if (node.dsubnodes != NULL) {
			const struct treecli_dnode *d;
			for (size_t j = 0; (d = (*(node.dsubnodes))[j]) != NULL; j++) {
				uint32_t i = 0;
				while (i < TREECLI_DNODE_MAX_COUNT) {
					char name[TREECLI_DNODE_MAX_NAME_LEN];
					if (treecli_parser_dnode_get_name(parser, d, i, name) != TREECLI_PARSER_DNODE_GET_NAME_OK) {
						break;
					}
					/* if the node was successfully created */
					if (treecli_parser_try_match(parser, matches, token, len, name) == TREECLI_PARSER_TRY_MATCH_OK) {
						matches->dsubnode = d;
						matches->dsubnode_index = i;
						ret = TREECLI_PARSER_GET_MATCHES_DSUBNODE;
					}
					i++;
				}
			}
		}

		/* Match values at current position/level. */
		if (node.values != NULL) {
			const struct treecli_value *v;
			for (size_t i = 0; (v = (*(node.values))[i]) != NULL; i++) {

				if (treecli_parser_try_match(parser, matches, token, len, v->name) == TREECLI_PARSER_TRY_MATCH_OK) {
					matches->value = v;
					parser->parsing_context = TREECLI_PARSER_CONTEXT_VALUE_OPERATOR;
					ret = TREECLI_PARSER_GET_MATCHES_VALUE;
				}
			}
		}

		/* And match commands at current position/level. */
		if (node.commands != NULL) {
			const struct treecli_command *c;
			for (size_t i = 0; (c = (*(node.commands))[i]) != NULL; i++) {

				if (treecli_parser_try_match(parser, matches, token, len, c->name) == TREECLI_PARSER_TRY_MATCH_OK) {
					matches->command = c;
					ret = TREECLI_PARSER_GET_MATCHES_COMMAND;
				}
			}
		}
	}

	if (matches->count == 1) {
		if (u_assert(ret != TREECLI_PARSER_GET_MATCHES_NONE)) {
			return TREECLI_PARSER_GET_MATCHES_FAILED;
		}
		return ret;
	} else if (matches->count > 1) {
		return TREECLI_PARSER_GET_MATCHES_MULTIPLE;
	}

	return TREECLI_PARSER_GET_MATCHES_NONE;
}


int32_t treecli_parser_pos_move(struct treecli_parser_pos *pos, struct treecli_parser_pos_level *level) {
	if (u_assert(pos != NULL) ||
	    u_assert(level != NULL)) {
		return TREECLI_PARSER_POS_MOVE_FAILED;
	}

	if (pos->depth < TREECLI_TREE_MAX_DEPTH) {
		memcpy(&(pos->levels[pos->depth]), level, sizeof(struct treecli_parser_pos_level));
		pos->depth++;

		return TREECLI_PARSER_POS_MOVE_OK;
	}

	return TREECLI_PARSER_POS_MOVE_FAILED;
}


int32_t treecli_parser_pos_up(struct treecli_parser_pos *pos) {
	if (u_assert(pos != NULL)) {
		return TREECLI_PARSER_POS_UP_FAILED;
	}

	if (pos->depth > 0) {
		pos->depth--;
		return TREECLI_PARSER_POS_UP_OK;
	}
	return TREECLI_PARSER_POS_UP_FAILED;
}


int32_t treecli_parser_pos_root(struct treecli_parser_pos *pos) {
	if (u_assert(pos != NULL)) {
		return TREECLI_PARSER_POS_ROOT_FAILED;
	}

	pos->depth = 0;

	return TREECLI_PARSER_POS_ROOT_OK;
}


int32_t treecli_parser_pos_copy(struct treecli_parser_pos *pos, struct treecli_parser_pos *src) {
	if (u_assert(pos != NULL) ||
	    u_assert(src != NULL)) {
		return TREECLI_PARSER_POS_COPY_FAILED;
	}

	memcpy(pos, src, sizeof(struct treecli_parser_pos));

	return TREECLI_PARSER_POS_COPY_OK;
}


int32_t treecli_parser_pos_init(struct treecli_parser_pos *pos) {
	if (u_assert(pos != NULL)) {
		return TREECLI_PARSER_POS_INIT_FAILED;
	}

	pos->depth = 0;

	return TREECLI_PARSER_POS_INIT_OK;
}


int32_t treecli_parser_get_current_node(struct treecli_parser *parser, struct treecli_node *node) {
	if (u_assert(parser != NULL) ||
	    u_assert(node != NULL)) {
		return TREECLI_PARSER_GET_CURRENT_NODE_FAILED;
	}

	struct treecli_parser_pos *pos = &(parser->pos);

	if (pos->depth == 0) {
		return TREECLI_PARSER_GET_CURRENT_NODE_ROOT;
	} else {
		if (pos->levels[pos->depth - 1].node != NULL) {

			memcpy(node, pos->levels[pos->depth - 1].node, sizeof(struct treecli_node));
			return TREECLI_PARSER_GET_CURRENT_NODE_OK;

		} else if (pos->levels[pos->depth - 1].dnode != NULL) {

			const struct treecli_dnode *d = pos->levels[pos->depth - 1].dnode;

			/* construct dynamic node */
			struct treecli_node dnode;
			memset(&dnode, 0, sizeof(dnode));

			if (d->create != NULL) {
				if (d->create(parser, pos->levels[pos->depth - 1].dnode_index, &dnode, d->create_context) >= 0) {
					memcpy(node, &dnode, sizeof(struct treecli_node));
					return TREECLI_PARSER_GET_CURRENT_NODE_OK;
				}
			}

			return TREECLI_PARSER_GET_CURRENT_NODE_FAILED;
		} else {
			return TREECLI_PARSER_GET_CURRENT_NODE_FAILED;
		}
	}

	return TREECLI_PARSER_GET_CURRENT_NODE_FAILED;
}


int32_t treecli_parser_help(struct treecli_parser *parser) {
	if (u_assert(parser != NULL)) {
		return TREECLI_PARSER_HELP_FAILED;
	}

	struct treecli_node node;
	int32_t res = treecli_parser_get_current_node(parser, &node);
	if (res == TREECLI_PARSER_GET_CURRENT_NODE_ROOT) {
		memcpy(&node, parser->top, sizeof(struct treecli_node));
	} else if (res == TREECLI_PARSER_GET_CURRENT_NODE_FAILED) {
		return TREECLI_PARSER_HELP_FAILED;
	}

	parser->print_handler(TREECLI_PARSER_AVAILABLE_SUBNODES, parser->print_handler_ctx);
	const struct treecli_node *n;
	for (size_t i = 0; (n = (*(node.subnodes))[i]) != NULL; i++) {
		parser->print_handler("\t", parser->print_handler_ctx);
		parser->print_handler(n->name, parser->print_handler_ctx);
		parser->print_handler(" - ", parser->print_handler_ctx);
		if (n->help != NULL) {
			parser->print_handler(n->help, parser->print_handler_ctx);
		} else {
			parser->print_handler(TREECLI_PARSER_HELP_UNAVAILABLE, parser->print_handler_ctx);
		}
		parser->print_handler("\n", parser->print_handler_ctx);
	}

	parser->print_handler(TREECLI_PARSER_AVAILABLE_COMMANDS, parser->print_handler_ctx);
	const struct treecli_command *c;
	for (size_t i = 0; (c = (*(node.commands))[i]) != NULL; i++) {
		parser->print_handler("\t", parser->print_handler_ctx);
		parser->print_handler(c->name, parser->print_handler_ctx);
		parser->print_handler(" - ", parser->print_handler_ctx);
		if (c->help != NULL) {
			parser->print_handler(c->help, parser->print_handler_ctx);
		} else {
			parser->print_handler(TREECLI_PARSER_HELP_UNAVAILABLE, parser->print_handler_ctx);
		}
		parser->print_handler("\n", parser->print_handler_ctx);
	}

	return TREECLI_PARSER_HELP_OK;
}


int32_t treecli_parser_dnode_get_name(struct treecli_parser *parser, const struct treecli_dnode *dnode, uint32_t index, char *name) {
	if (u_assert(parser != NULL) ||
	    u_assert(dnode != NULL) ||
	    u_assert(name != NULL)) {
		return TREECLI_PARSER_DNODE_GET_NAME_FAILED;
	}

	/* allocate node structure and name */
	struct treecli_node node;
	memset(&node, 0, sizeof(node));
	node.name = name;

	/* default name is created */
	sprintf(node.name, "%s%d", dnode->name, (int)index);

	if (dnode->create != NULL && dnode->create(parser, index, &node, dnode->create_context) >= 0) {
		return TREECLI_PARSER_DNODE_GET_NAME_OK;
	}

	return TREECLI_PARSER_DNODE_GET_NAME_FAILED;
}


int32_t treecli_parser_set_mode(struct treecli_parser *parser, enum treecli_parser_mode mode) {
	if (u_assert(parser != NULL)) {
		return TREECLI_PARSER_SET_MODE_FAILED;
	}

	parser->mode = mode;

	return TREECLI_PARSER_SET_MODE_OK;
}


int32_t treecli_parser_value_to_str(struct treecli_parser *parser, char *s, const struct treecli_value *value, uint32_t max) {
	if (u_assert(parser != NULL) ||
	    u_assert(s != NULL) ||
	    u_assert(value != NULL) ||
	    u_assert(value->value != NULL)) {
		return TREECLI_PARSER_VALUE_TO_STR_FAILED;
	}

	/* TODO: time and date */
	switch (value->value_type) {
		case TREECLI_VALUE_INT32:
			snprintf(s, max, "%ld", *(int32_t *)value->value);
			s[max - 1] = '\0';
			break;

		case TREECLI_VALUE_UINT32:
			snprintf(s, max, "%lu", *(uint32_t *)value->value);
			s[max - 1] = '\0';
			break;

		case TREECLI_VALUE_STR:
			strncpy(s, (char *)value->value, max);
			s[max - 1] = '\0';
			break;

		case TREECLI_VALUE_PHYS:
			if (u_assert(value->units != NULL)) {
				return TREECLI_PARSER_VALUE_TO_STR_FAILED;
			}
			snprintf(s, max, "%ld%s", *(int32_t *)value->value, value->units);
			s[max - 1] = '\0';
			break;

		case TREECLI_VALUE_DATA:
			if ((*(uint32_t *)value->value) > (1024 * 1024)) {
				snprintf(s, max, "%luMiB", (*(uint32_t *)value->value) / 1024 / 1024);
			} else if ((*(uint32_t *)value->value) > (1024)) {
				snprintf(s, max, "%luKiB", (*(uint32_t *)value->value) / 1024);
			} else {
				snprintf(s, max, "%luB", *(uint32_t *)value->value);
			}
			break;

		default:
			return TREECLI_PARSER_VALUE_TO_STR_FAILED;
	}


	return TREECLI_PARSER_VALUE_TO_STR_OK;
}


int32_t treecli_parser_str_to_value(struct treecli_parser *parser, const struct treecli_value *value, const char *s, uint32_t len) {
	if (u_assert(parser != NULL) ||
	    u_assert(s != NULL) ||
	    u_assert(value != NULL)) {
		return TREECLI_PARSER_STR_TO_VALUE_FAILED;
	}

	switch (value->value_type) {
		case TREECLI_VALUE_INT32: {
				int32_t v = 0;
				bool negative = false;
				for (uint32_t i = 0; i < len; i++) {
					if (s[i] == '-') {
						negative = true;
					} else if (s[i] >= '0' && s[i] <= '9') {
						v = v * 10 + (s[i] - '0');
					} else {
						return TREECLI_PARSER_STR_TO_VALUE_FAILED;
					}
				}
				if (negative) {
					v = -v;
				}
				if (value->value != NULL) {
					*(int32_t *)value->value = v;
				}
				if (value->set != NULL) {
					value->set(parser, value->get_set_context, value, (void *)&v, 4);
				}
			}
			break;

		case TREECLI_VALUE_UINT32: {
				uint32_t v = 0;
				for (uint32_t i = 0; i < len; i++) {
					if (s[i] >= '0' && s[i] <= '9') {
						v = v * 10 + (s[i] - '0');
					} else {
						return TREECLI_PARSER_STR_TO_VALUE_FAILED;
					}
				}
				if (value->value != NULL) {
					*(uint32_t *)value->value = v;
				}
				if (value->set != NULL) {
					value->set(parser, value->get_set_context, value, (void *)&v, 4);
				}
			}
			break;

		case TREECLI_VALUE_STR: {
				if (value->set != NULL) {
					/* Strip the leading and trailing double qoutes. */
					if (s[0] == '"' && s[len - 1] == '"') {
						s += 1;
						len -= 2;
					}
					value->set(parser, value->get_set_context, value, (void *)s, len);
				}
			}
			break;

		case TREECLI_VALUE_BOOL: {
				bool v = false;
				if (len == 1) {
					if (s[0] == 'f' || s[0] == 'F' || s[0] == '0' || s[0] == 'n' || s[0] == 'N') {
						v = false;
					}
					if (s[0] == 't' || s[0] == 'T' || s[0] == '1' || s[0] == 'y' || s[0] == 'Y') {
						v = true;
					}
				}
				if (len == 2 && (strncmp(s, "no", 2) != 0)) {
					v = false;
				}
				if (len == 3 && (strncmp(s, "yes", 3) != 0)) {
					v = true;
				}
				if (len == 4 && (strncmp(s, "true", 4) != 0)) {
					v = true;
				}
				if (len == 5 && (strncmp(s, "false", 5) != 0)) {
					v = false;
				}

				if (value->set != NULL) {
					value->set(parser, value->get_set_context, value, (void *)&v, 4);
				}
			}
			break;

		default:
			return TREECLI_PARSER_VALUE_TO_STR_FAILED;
	}

	return TREECLI_PARSER_STR_TO_VALUE_OK;
}


int32_t treecli_parser_set_context(struct treecli_parser *parser, void *context) {
	if (u_assert(parser != NULL && context != NULL)) {
		return TREECLI_PARSER_SET_CONTEXT_FAILED;
	}

	parser->context = context;

	return TREECLI_PARSER_SET_CONTEXT_OK;
}
