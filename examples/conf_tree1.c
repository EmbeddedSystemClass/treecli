/* Sample configuration tree structure. All nodes are written in inverted hierarchy,
 * top nodes being at the bottom (upper nodes are referencing their subnodes). */


/* Global variables to be manipulated during configuration */
uint32_t test_value;


/**************************** /interface commands *****************************/

int32_t test1_interface_print_exec(struct treecli_parser *parser, void *exec_context) {
	printf("printcommand issued, command context = %d\n", exec_context);
};

const struct treecli_command test1_interface_print = {
	.name = "print",
	.exec = test1_interface_print_exec,
	.exec_context = (void *)1234,
	.next = NULL
};


/*************************** /interface/ifN values ****************************/

const struct treecli_value test1_interface_ifN_enabled = {
	.name = "enabled",
	.value = &test_value,
	.value_type = TREECLI_VALUE_UINT32,
	.default_value = &(int){1234},
	.next = NULL
};


/***************************** /interface dnodes ******************************/

int32_t test1_interface_ifN_create(struct treecli_parser *parser, uint32_t index, struct treecli_node *node, void *ctx) {

	if (index <= 6) {
		if (node->name != NULL) {
			sprintf(node->name, "ethernet%d", index);
		}
		node->values = &test1_interface_ifN_enabled;
		return 0;
	}

	return -1;
}

const struct treecli_dnode test1_interface_ifN = {
	.name = "if",
	.create = test1_interface_ifN_create,
};


/************************* /system/bootloader values **************************/

const struct treecli_value test1_system_bootloader_postbootaction = {
	.name = "postboot-action",
	.next = NULL
};

const struct treecli_value test1_system_bootloader_image = {
	.name = "image",
	.next = &test1_system_bootloader_postbootaction
};

const struct treecli_value test1_system_bootloader_backupimage = {
	.name = "backup-image",
	.next = &test1_system_bootloader_image
};

const struct treecli_value test1_system_bootloader_prebootdelay = {
	.name = "preboot-delay",
	.next = &test1_system_bootloader_backupimage
};

const struct treecli_value test1_system_bootloader_verifyimage = {
	.name = "verify-image",
	.next = &test1_system_bootloader_prebootdelay
};

const struct treecli_value test1_system_bootloader_consolespeed = {
	.name = "console-speed",
	.next = &test1_system_bootloader_verifyimage
};


/***************************** /system commands *******************************/

int32_t test1_system_quit_exec(struct treecli_parser *parser, void *exec_context) {
	quit_req = 1;
};

const struct treecli_command test1_system_quit = {
	.name = "quit",
	.help = "Exit the whole thing",
	.exec = test1_system_quit_exec,
};


/***************************** /system subnodes *******************************/

const struct treecli_node test1_system_bootloader = {
	.name = "bootloader",
	.values = &test1_system_bootloader_consolespeed
};


/******************************* /power values ********************************/



/****************************** /power commands *******************************/

const struct treecli_command test1_power_reboot = {
	.name = "reboot",
	.next = NULL
};

const struct treecli_command test1_power_poweroff = {
	.name = "poweroff",
	.next = &test1_power_reboot
};


/****************************** /power subnodes *******************************/

const struct treecli_node test1_power_source = {
	.name = "source",
};


/******************************* root subnodes ********************************/

const struct treecli_node test1_power = {
	.name = "inpower",
	.help = "System power manipulation (poweroff, reboot, power mode)",
	.commands = &test1_power_poweroff,
	.subnodes = &test1_power_source
};

const struct treecli_node test1_interface = {
	.name = "interface",
	.commands = &test1_interface_print,
	.dsubnodes = &test1_interface_ifN,
	.next = &test1_power
};

const struct treecli_node test1_system = {
	.name = "system",
	.help = "Various commands for system management",
	.commands = &test1_system_quit,
	.subnodes = &test1_system_bootloader,
	.next = &test1_interface
};


/********************************* root node **********************************/
const struct treecli_node test1 = {
	.name = "/",
	.subnodes = &test1_system
};

