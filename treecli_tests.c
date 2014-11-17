#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "treecli_parser.h"

static int test_treecli_token_get(void) {
	
	char *line1 = "  fiRSt \t  tok3n test ";
	char *token;
	int32_t len;

	if (treecli_token_get(NULL, &line1, &token, &len) != TREECLI_TOKEN_GET_OK) return 0;
	if (strncmp(token, "fiRSt", 5)) return 0;
	if (len != 5) return 0;

	if (treecli_token_get(NULL, &line1, &token, &len) != TREECLI_TOKEN_GET_OK)  return 0;
	if (strncmp(token, "tok3n", 5)) return 0;
	if (len != 5) return 0;

	if (treecli_token_get(NULL, &line1, &token, &len) != TREECLI_TOKEN_GET_OK) return 0;
	if (strncmp(token, "test", 4)) return 0;
	if (len != 4) return 0;
	
	if (treecli_token_get(NULL, &line1, &token, &len) != TREECLI_TOKEN_GET_NONE) return 0;

	char *line2 = "  fiRSt \t  tok3n test ";

	return 1;
}

static void test_result(char *name, int result) {
	printf("test %s: %s\n", name, result ? "OK" : "FAILED");
}


int treecli_run_tests(void) {
	test_result("treecli_token_get", test_treecli_token_get());
	
}
