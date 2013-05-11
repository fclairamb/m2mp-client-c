/*
 * File:   dictionnary.c
 * Author: florent
 *
 * Created on Apr 14, 2012, 2:53:10 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include "CUnit/Basic.h"
#include "dictionnary.h"
#include "memwatcher.h"

/*
 * CUnit Test Suite
 */

int init_suite(void) {
	return 0;
}

int clean_suite(void) {
	return 0;
}

void testDictionnary_new() {
	mw_init();
	
	dictionnary* dic = dictionnary_new();

	dictionnary_add( dic, "Florent", "Clairambault" );
	dictionnary_add( dic, "Banksy", "Brainwash");
	
	dictionnary_delete( dic );
	
	mw_end();
}

int main() {
	CU_pSuite pSuite = NULL;

	/* Initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	/* Add a suite to the registry */
	pSuite = CU_add_suite("dictionnary", init_suite, clean_suite);
	if (NULL == pSuite) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* Add the tests to the suite */
	if ((NULL == CU_add_test(pSuite, "testDictionnary_new", testDictionnary_new))) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* Run all tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}
