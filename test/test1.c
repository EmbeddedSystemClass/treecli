#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "treecli_parser.h"
#include "treecli_shell.h"
#include "treecli_tests.h"
#include "lineedit.h"


/**
 * Configured values
 */
uint32_t test_value;



/**
 * Configuration tree structure
 */

int32_t test1_interface_print_exec(struct treecli_parser *parser, void *exec_context) {
	printf("nieco, ctx = %d\n", exec_context);
};

const struct treecli_command test1_interface_print = {
	.name = "print",
	.exec = test1_interface_print_exec,
	.exec_context = (void *)1234,
	.next = NULL
};

const struct treecli_value test1_interface_enabled = {
	.name = "enabled",
	.value = &test_value,
	.value_type = TREECLI_VALUE_UINT32,
	.default_value = &(int){1234},
	.next = NULL
};

const struct treecli_node test1_interface = {
	.name = "interface",
	.values = &test1_interface_enabled,
	.commands = &test1_interface_print,
	.next = NULL
};

const struct treecli_node test1_system = {
	.name = "system",
	.next = &test1_interface
};

const struct treecli_node test1 = {
	.name = "/",
	.subnodes = &test1_system
};




int32_t parser_output(const char *s, void *ctx) {
	printf("%s", s);
	return 0;
}



int main(int argc, char *argv[]) {

	//~ treecli_print_tree(&test1, 0);
	//~ treecli_run_tests();
	
	struct treecli_shell sh;
	treecli_shell_init(&sh, &test1);
	treecli_shell_set_print_handler(&sh, parser_output, (void *)&sh);

	while (!feof(stdin)) {
		int c = fgetc(stdin);
		
		int32_t ret = treecli_shell_keypress(&sh, c);
	}
	
	treecli_shell_free(&sh);
}

