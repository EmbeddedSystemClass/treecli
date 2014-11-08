#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "treecli.h"
#include "treecli_tests.h"

/**
 * Configured values
 */
uint32_t test_value;



/**
 * Configuration tree structure
 */

const struct treecli_value test1_interface_enabled = {
	.name = "enabled",
	.value = &test_value,
	.value_type = TREECLI_VALUE_UINT32,
	.default_value = &(int){1234},
	.next = NULL
};

const struct treecli_node test1_interface = {
	.name = "interface",
	.next = NULL
};

const struct treecli_node test1_system = {
	.name = "system",
	.next = &test1_interface
};

const struct treecli_node test1 = {
	.name = "/",
	.subnodes = &test1_system,
	.values = &test1_interface_enabled
};




int main(int argc, char *argv[]) {

	//~ treecli_print_tree(&test1, 0);

	treecli_run_tests();
}

