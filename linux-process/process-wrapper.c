// weibo上有人说到https://github.com/joewalnes/websocketd，我想其原理大致如此（process-wrapper.c）:

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int wrap_process(const char* exefile) {
  int status;
  pid_t pid;
  char* output_file = "./wrapped-output.txt";
  int output_fd;

  output_fd = open(output_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (output_fd == -1) {
    return -1;
  }

  pid = fork();
  if (pid < 0) {
    return 0;
  }
  else if (pid == 0) { // child process
    dup2(output_fd, 1);
    close(output_fd);

    execlp(exefile, exefile, NULL);
    exit(0);
  }
  else { // parent process
    waitpid(pid, &status, 0);
    return 0;
  }
}

int main(int argc, char *argv[]) {
  int ret;

  if (argc != 2) {
    printf("Usage: ./process-wrapper $PROG_NAME");
    return -1;
  }

  ret = wrap_process(argv[1]);

  exit(ret);
}
