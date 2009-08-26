#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>

#include <conflate.h>

#include "test_common.h"

void check_pair_equality(kvpair_t *one, kvpair_t *two)
{
    fail_if(one == NULL, "one is null.");
    fail_if(one == two, "Comparing identical pairs.");
    fail_unless(strcmp(one->key, two->key) == 0, "Keys don't match.");

    int i = 0;
    for (i=0; one->values[i]; i++) {
        fail_unless(two->values[i] != NULL, "Value at one, but not two");
        fail_unless(strcmp(one->values[i], two->values[i]) == 0,
                    "Values don't match.");
        fail_unless((one->values[i+1] != NULL && two->values[i+1] != NULL)
                    || (one->values[i+1] == NULL && two->values[i+1] == NULL),
                    "Unbalanced values.");

    }

    if (one->next) {
        fail_unless(two->next != NULL, "No two->next.");
        check_pair_equality(one->next, two->next);
    } else {
        fail_if(two->next, "No one->next, but a two->next");
    }
}
