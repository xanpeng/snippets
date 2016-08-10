#include "list.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct data {
    int elem;
    struct list_head list;
} data_t;

LIST_HEAD(head_1);
LIST_HEAD(head_2);

void create_list(struct list_head *head, int start, int finish)
{
    int i;
    for (i = start; i <= finish; ++i) {
        data_t *d = malloc(sizeof(data_t));
        d->elem = i,
        list_add_tail(&d->list, head);  // queue
        // list_add(&d->list, head);    // stack
    }
}

void print_list(struct list_head *head)
{
    struct list_head *iter;
    data_t *dptr;

    list_for_each(iter, head) {
        dptr = list_entry(iter, data_t, list);
        printf("%d->", dptr->elem);
    }
    /*    list_for_each_entry(dptr, head, list)        printf("%d->", dptr->elem);    */
    printf("\n");
}

int main()
{
    create_list(&head_1, 1, 5);
    print_list(&head_1);

    create_list(&head_2, 6, 10);
    print_list(&head_2);

    // list_move_tail(&head_2, &head_1);
    list_splice(&head_1, &head_2);
    print_list(&head_1);

    return 0;
}
