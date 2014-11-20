#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "treecli_parser.h"
#include "treecli_shell.h"
#include "lineedit.h"


uint32_t quit_req = 0;

/* This file contains example configuration tree (definitions of all nodes,
 * subnodes, commands and values together with global variables which will be
 * manipulated during configuration. This simple setup is enough for most cases. */
#include "conf_tree1.c"

/* We only need to define single callback function to provide output channel
 * for the parser. */
int32_t parser_output(const char *s, void *ctx) {
	printf("%s", s);
	return 0;
}


int main(int argc, char *argv[]) {

	/* Initialize the shell and set print callback */
	struct treecli_shell sh;
	treecli_shell_init(&sh, &test1);
	treecli_shell_set_print_handler(&sh, parser_output, (void *)&sh);

	/* loop while we have something to read from the input */
	while (!feof(stdin)) {
		int c = fgetc(stdin);

		/* pass keypresses directly to the treecli library */
		int32_t ret = treecli_shell_keypress(&sh, c);

		/* quit_req variable is set by an exec callback of one of the
		 * commands in the configuration tree. */
		if (quit_req) {
			break;
		}
	}

	/* don't forget to free the shell context */
	treecli_shell_free(&sh);
}

