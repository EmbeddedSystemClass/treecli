#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "treecli.h"
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

int32_t prompt(struct lineedit *le, void *ctx) {
	treecli_parser_pos_print(&(((struct treecli_parser *)ctx)->pos));
	
	le->print_handler(" > ", le->print_handler_ctx);
}


int main(int argc, char *argv[]) {

	//~ treecli_print_tree(&test1, 0);
	//~ treecli_run_tests();
	
	struct treecli_parser parser;
	treecli_parser_init(&parser, &test1);
	treecli_parser_set_print_handler(&parser, parser_output, NULL);
	parser.allow_exec = 1;
	treecli_parser_parse_line(&parser, "in");

	struct lineedit line;
	
	lineedit_init(&line, 200);
	lineedit_set_print_handler(&line, parser_output, NULL);
	lineedit_set_prompt_callback(&line, prompt, (void *)&parser);

	lineedit_set_line(&line, "abcd");
	lineedit_set_cursor(&line, 4);
	lineedit_clear(&line);
	lineedit_refresh(&line);

	while (!feof(stdin)) {
		int c = fgetc(stdin);
		
		int32_t ret = lineedit_keypress(&line, c);
		
		if (ret == LINEEDIT_ENTER) {
			break;
		}
	}
	
	char *text;
	lineedit_get_line(&line, &text);
	printf("\n\nline='%s'\n", text);

	lineedit_free(&line);
	treecli_parser_free(&parser);
}

