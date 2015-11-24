#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdbool.h>
#include <unistd.h>

union semun {
		int val;
		struct semid_ds *buf;
		unsigned short int *array;
		struct seminfo *__buf;
} semarg;

key_t semkey;
int semid;

static bool inline sem_op(bool up)
{
		struct sembuf ops;
		ops.sem_num = 0;
		ops.sem_flg = IPC_NOWAIT | SEM_UNDO;
		ops.sem_op = up ? -1 : 1;
		if (semop(semid, &ops, 1) < 0) {
				fprintf(stderr, "semop %s failed: %m\n", up ? "up" : "down");
				return false;
		}
		return true;
}

static bool sem_up(void)
{
		return sem_op(true);
}

static bool sem_down(void)
{
		return sem_op(false);
}

static void do_work(void)
{
		fprintf(stderr, "child: try sem_up and sleep\n");
		sem_up();
		sleep(10);
		sem_down();
}

int main()
{
		semkey = random();
		semid = semget(semkey, 1, 0666 | IPC_CREAT);
		if (semid < 0) {
				fprintf(stderr, "semget failed: %m");
				exit(1);
		}

		semarg.val = 1;
		if (semctl(semid, 0, SETVAL, semarg) < 0) {
				fprintf(stderr, "semctl failed: %m");
				exit(1);
		}

		if (fork()) {
				sleep(1);
				fprintf(stderr, "try semup\n");
				if (sem_up()) {
						fprintf(stderr, "semup succeed\n");
						sem_down();
				}
				exit(1);
		} else {
				do_work();
		}
}
