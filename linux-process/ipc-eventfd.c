#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/eventfd.h>

typedef void (*event_handler_t)(int fd, int events, void *data);

struct event_info {
    event_handler_t handler;
    int fd;
    void *data;
    int prio;
};

int epoll_fd;
struct epoll_event *events;
int rw_efd = 0;

static void event_handler(int listen_fd, int events, void *data)
{
    int ret;
    long val = 0;

    ret = read(rw_efd, &val, sizeof(long));
    printf("read value=%d,ret=%d,\n", val, ret);
}

static void efd_write(void)
{
    int ret;
    long val = 5;

    ret = write(rw_efd, &val, sizeof(long));
    printf("efd write val=%d,ret=%d,err=%s\n", val, ret, strerror(errno));
}

int main()
{
    int ret;
    int nr;
    int i;
    struct epoll_event ev;
    struct event_info *ei;    

    epoll_fd = epoll_create(128);
    events = calloc(128, sizeof(struct epoll_event));
    
    rw_efd = eventfd(0, EFD_NONBLOCK);    
   
    ei = malloc(sizeof(struct event_info));
    ei->fd = rw_efd;
    ei->handler = event_handler;
    ei->data = NULL;
    ei->prio = 0;
    ev.events = EPOLLIN;
    ev.data.ptr = ei;
    
    efd_write(); // efd write val=5,ret=8,err=Success
    efd_write(); // efd write val=5,ret=8,err=Success
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, rw_efd, &ev); 
    efd_write(); // efd write val=5,ret=8,err=Success

    while (1) {
        nr = epoll_wait(epoll_fd, events, 128, -1); // read value=15,ret=8,
        for (i = 0; i < nr; i++) {
            struct event_info *ei;

            ei = (struct event_info *)events[i].data.ptr;
            ei->handler(ei->fd, events[i].events, ei->data);
        } 
    }
}

