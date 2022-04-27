#include <stdio.h>

#include "libsprintpcb/list.h"
#include "libsprintpcb/primitives.h"
#include "libsprintpcb/elements.h"

int main() {
    sprint_tuple tuple1, tuple2;
    tuple1.x = 1;
    tuple1.y = 2;
    tuple2.x = 3;
    tuple2.y = 4;

    printf("track %d\n", sizeof(sprint_track));
    printf("pad_tht %d\n", sizeof(sprint_pad_tht));
    printf("pad_smt %d\n", sizeof(sprint_pad_smt));
    printf("zone %d\n", sizeof(sprint_zone));
    printf("text %d\n", sizeof(sprint_text));
    printf("circle %d\n", sizeof(sprint_circle));
    printf("component %d\n", sizeof(sprint_component));
    printf("group %d\n", sizeof(sprint_group));
    printf("\nelement %d\n", sizeof(sprint_element));

    sprint_list* list = sprint_list_create(sizeof(sprint_tuple), 0);
    sprint_list_add(list, &tuple1);
    sprint_list_add(list, &tuple2);
    sprint_list_add(list, &tuple2);

    for (int i = 0; i < list->count; i++) {
        sprint_tuple* tuple = (sprint_tuple*)sprint_list_get(list, i);
        printf("%d / %d\n", tuple->x, tuple->y);
    }

    sprint_list_destroy(list);

    return 0;
}
