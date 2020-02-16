#include <stdio.h>
#include "minunit.h"
#include "utils\LinkList.c"

int tests_run = 0;

static char* add_192_last_to_empty_list_makes_its_first_element_192() {
	int myValue = 192, myReturn;
	LinkList* list = LinkList_alloc();
	LinkList_addLast(list, &myValue);
	myReturn = *(int*)LinkList_getFirst(list);
	LinkList_free(list);
	mu_assert("Adding 192 last to empty list should have made 192 first.", myValue == myReturn);
	return 0;
}

static char* add_128_last_to_list_with_192_first_makes_first_element_192_and_last_128() {
	int value1 = 192, value2 = 128, return1, return2;
	LinkList* list = LinkList_alloc();
	LinkList_addLast(list, &value1);
	LinkList_addLast(list, &value2);
	return1 = *(int*)LinkList_getFirst(list);
	return2 = *(int*)LinkList_getLast(list);
	LinkList_free(list);
	mu_assert("Adding 128 last to a list with 192 Should have been 192 first and 128 last", return1 == 192 && return2 == 128);
	return 0;
}

static char* add_128_first_to_list_with_192_first_makes_first_element_128_and_last_192() {
	int value1 = 192, value2 = 128, return1, return2;
	LinkList* list = LinkList_alloc();
	LinkList_addLast(list, &value1);
	LinkList_addFirst(list, &value2);
	return1 = *(int*)LinkList_getFirst(list);
	return2 = *(int*)LinkList_getLast(list);
	LinkList_free(list);
	mu_assert("Adding 128 first to a list with 192 Should have been 128 first and 192 last", return1 == 128 && return2 == 192);
	return 0;
}

static char* add_45_at_1_in_list_with_30_and_60_makes_element_1_be_45() {
	int value1 = 30, value2 = 60, value3=45, returnValue;
	LinkList* list = LinkList_alloc();
	LinkList_addLast(list, &value1);
	LinkList_addLast(list, &value2);
	LinkList_addAt(list, &value3, 1);
	returnValue = *(int*)LinkList_getAt(list, 1);
	LinkList_free(list);
	mu_assert("Adding 45 at 1 to a list with 30 and 60 should and getting 1 should have been 45", returnValue == 45);
	return 0;
}

static char* removing_element_1_in_list_with_30_45_60_makes_element_1_be_60() {
	int value1 = 30, value2 = 60, value3=45, returnValue;
	LinkList* list = LinkList_alloc();
	LinkList_addLast(list, &value1);
	LinkList_addLast(list, &value3);
	LinkList_addLast(list, &value2);
	LinkList_removeAt(list, 1);
	returnValue = *(int*)LinkList_getAt(list, 1);
	LinkList_free(list);
	mu_assert("Removing 45 from a list with 30 45 and 60 should have made element 1 60", returnValue == 60);
	return 0;
}

static char* removing_first_in_list_with_30_45_60_makes_element_0_be_45() {
	int value1 = 30, value2 = 60, value3=45, returnValue;
	LinkList* list = LinkList_alloc();
	LinkList_addLast(list, &value1);
	LinkList_addLast(list, &value3);
	LinkList_addLast(list, &value2);
	LinkList_removeFirst(list);
	returnValue = *(int*)LinkList_getAt(list, 0);
	LinkList_free(list);
	mu_assert("Removing first element from a list with 30 45 and 60 should have made element 0 45", returnValue == 45);
	return 0;
}

static char* removing_last_in_list_with_30_45_60_makes_last_element_be_45() {
	int value1 = 30, value2 = 60, value3=45, returnValue;
	LinkList* list = LinkList_alloc();
	LinkList_addLast(list, &value1);
	LinkList_addLast(list, &value3);
	LinkList_addLast(list, &value2);
	LinkList_removeLast(list);
	returnValue = *(int*)LinkList_getLast(list);
	LinkList_free(list);
	mu_assert("Removing first element from a list with 30 45 and 60 should have made element 0 45", returnValue == 45);
	return 0;
}

static char* setting_element_1_to_90_in_list_with_30_45_60_makes_element_1_90() {
	int value1 = 30, value2 = 60, value3=45, value4=90, returnValue;
	LinkList* list = LinkList_alloc();
	LinkList_addLast(list, &value1);
	LinkList_addLast(list, &value3);
	LinkList_addLast(list, &value2);
	LinkList_setAt(list, 1, &value4);
	returnValue = *(int*)LinkList_getAt(list, 1);
	LinkList_free(list);
	mu_assert("Setting first element from a list with 30 45 and 60 to 90 should have made element 1 90", returnValue == 90);
	return 0;
}

static char* the_size_of_a_list_with_8_elements_should_return_8() {
	int randomvalue = 0, len = 0;
	LinkList* list = LinkList_alloc();
	LinkList_addLast(list, &randomvalue);
	LinkList_addLast(list, &randomvalue);
	LinkList_addLast(list, &randomvalue);
	LinkList_addLast(list, &randomvalue);
	LinkList_addLast(list, &randomvalue);
	LinkList_addLast(list, &randomvalue);
	LinkList_addLast(list, &randomvalue);
	LinkList_addLast(list, &randomvalue);
	len = LinkList_listSize(list);
	LinkList_free(list);
	mu_assert("A list with 8 elements should have a size of 8", len == 8);
	return 0;
}

static char* a_list_with_8_elements_that_gets_cleared_should_have_no_elements() {
	int randomvalue = 0, len = 0;
	LinkList* list = LinkList_alloc();
	LinkList_addLast(list, &randomvalue);
	LinkList_addLast(list, &randomvalue);
	LinkList_addLast(list, &randomvalue);
	LinkList_addLast(list, &randomvalue);
	LinkList_addLast(list, &randomvalue);
	LinkList_addLast(list, &randomvalue);
	LinkList_addLast(list, &randomvalue);
	LinkList_addLast(list, &randomvalue);
	LinkList_clearList(list);
	len = LinkList_listSize(list);
	LinkList_free(list);
	mu_assert("A list with 8 elements that gets cleared should have a size of 0", len == 0);
	return 0;
}

static char* a_list_that_is_1_2_3_4_5_6_7_8_should_return_the_index_of_4_as_3() {
	int one = 1, two=2, three=3, four=4, five=5, six=6, seven=7, eight=8, returnIndex = 0;
	LinkList* list = LinkList_alloc();
	LinkList_addLast(list, &one);
	LinkList_addLast(list, &two);
	LinkList_addLast(list, &three);
	LinkList_addLast(list, &four);
	LinkList_addLast(list, &five);
	LinkList_addLast(list, &six);
	LinkList_addLast(list, &seven);
	LinkList_addLast(list, &eight);
	returnIndex = LinkList_listContains(list, &four);
	LinkList_free(list);
	mu_assert("A list from 1 to 8 should return the index of 4 as 3", returnIndex == 3);
	return 0;
}

static char* all_tests() {
	mu_run_test(add_192_last_to_empty_list_makes_its_first_element_192);
	mu_run_test(add_128_last_to_list_with_192_first_makes_first_element_192_and_last_128);
	mu_run_test(add_128_first_to_list_with_192_first_makes_first_element_128_and_last_192);
	mu_run_test(add_45_at_1_in_list_with_30_and_60_makes_element_1_be_45);
	mu_run_test(removing_element_1_in_list_with_30_45_60_makes_element_1_be_60);
	mu_run_test(removing_first_in_list_with_30_45_60_makes_element_0_be_45);
	mu_run_test(setting_element_1_to_90_in_list_with_30_45_60_makes_element_1_90);
	mu_run_test(the_size_of_a_list_with_8_elements_should_return_8);
	mu_run_test(a_list_with_8_elements_that_gets_cleared_should_have_no_elements);
	mu_run_test(a_list_that_is_1_2_3_4_5_6_7_8_should_return_the_index_of_4_as_3);
	return 0;
}

int main() {
	char *result = all_tests();
	if(result != 0) {
		printf("%s\n", result);
	}
	else {
		printf("ALL TESTS PASSED\n");
	}
	
	printf("Tests run: %d\n", tests_run);
	
	return result != 0;
}