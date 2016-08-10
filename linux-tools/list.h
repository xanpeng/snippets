// 多了一个临时变量，用来存储当前节点。
/** 
* list_for_each_safe - iterate over a list safe against removal of list entry 
* @pos:   the &struct list_head to use as a loop cursor. 
* @n:     another &struct list_head to use as temporary storage 
* @head:  the head for your list. */
#define list_for_each_safe(pos, n, head) \
   for (pos = (head)->next, n = pos->next; pos != (head); \
       pos = n, n = pos->next)#define list_for_each(pos, head) \
   for (pos = (head)->next; prefetch(pos->next), pos != (head); \
       pos = pos->next)
