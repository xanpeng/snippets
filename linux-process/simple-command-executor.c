/*
   A very simple and specified command executor.

usage: ./command-executor /path/to/commands-file

ATTENTION: it's not full-tested!

why: 
(1) you want test a bunch of programs; 
(2) you have a expected return value, you got a real return value;
(3) I'm not familiar with bash/python programming;
(4) bash, system() and popen() cannot retrieve subprocess exit status;
(5) I want to depress stdout+stderr of subprocess.

what's funny:
(1) fork + execvp(automatically know /path/to/executable);
(2) dup(stdout, fd) to redirect stdout, and the same with stderr;
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define err_exit(fmt, args...) do { \
  fprintf(stderr, fmt, ##args);   \
  exit(EXIT_FAILURE);             \
} while (0);

#define debug(fmt, args...) do {    \
  fprintf(stderr, fmt, ##args);   \
} while (0);

#define exelog "./functester.log"
#define proglen 32
#define argcnt 32   // how many args
#define arglen proglen   // maxlen of each args
#define linelen (proglen + (argcnt * arglen) + 8)

typedef struct _command {
  // prepared for fork()+execv()
  char prog[proglen];
  char *args[argcnt];
  int argc;
  int expect_ret;
  int real_ret;
  struct _command *next;
} command_t;
command_t *all_cmds;

// *line must be modifiable
static void construct_one_cmd(command_t *cmd, char *line)
{
  // command file entry format: "expected_val command"
  // space delimited
  int argc = 0;
  const char *delim = " \t";
  char *iter;

  debug("...[expect + command] ");
  iter = strtok(line, delim);
  while (iter) {
    ++argc;
    if (argc == 1) {
      cmd->expect_ret = atoi(iter);
      debug("%d", cmd->expect_ret);
    }
    else if (argc == 2) {
      strncpy(cmd->prog, iter, proglen);
      debug(" %s", cmd->prog);

      cmd->args[argc-2] = strdup(iter);
      if (!cmd->args[argc-2]) err_exit("strdup() fail\n");
      debug(" %s", cmd->args[argc-2]);
    }
    else {
      cmd->args[argc-2] = strdup(iter);
      if (!cmd->args[argc-2]) err_exit("strdup() fail\n");
      debug(" %s", cmd->args[argc-2]);
    }
    iter = strtok(NULL, delim);
  }
  ++argc;
  cmd->args[argc-2] = NULL;
  cmd->argc = argc - 1;
  debug("\n");
}

static void construct_all_cmds(const char *cmds_file)
{
  FILE *fp = fopen(cmds_file, "r");
  if (!fp) err_exit("fopen fail\n");

  all_cmds = (command_t *) malloc(sizeof(command_t));
  if (!all_cmds) err_exit("malloc for all_cmds fail\n");
  all_cmds->next = NULL;

  command_t *curr = all_cmds;
  command_t *prev = curr;

  debug("constructing commands from [%s]\n", cmds_file);
  char line[linelen];
  int len;
  while (fgets(line, linelen, fp)) {
    if (!curr) {
      curr= (command_t *) malloc(sizeof(command_t));
      if (!curr) err_exit("malloc for next command_t fail\n");
      prev->next = curr;
    }

    if ('#' == line[0]) continue;
    len = strlen(line);
    line[len-1] = 0;

    construct_one_cmd(curr, line);
    prev = curr;
    curr = curr->next;
  }
  fclose(fp);
}

static void check_executed_result(command_t *curr)
{
  char line[linelen];
  int i;
  int len;

  snprintf(line, linelen, "%s", curr->prog);
  len = strlen(curr->prog);
  for (i = 1; i < curr->argc; ++i) {
    if (!curr->args[i]) break;
    snprintf(line + len, linelen, " %s", curr->args[i]);
    len = len + strlen(curr->args[i]) + 1;
  }

  debug("\ncommand: [%s]\n expect: %d\n   real: %d\n\n",
      line, curr->expect_ret, -curr->real_ret);
  if (curr->expect_ret != -curr->real_ret)
    exit(EXIT_FAILURE);
}

static int execute(command_t *cmd)
{
  pid_t pid = fork();
  if (pid < 0) err_exit("fork fail\n");

  if (!pid) {
    int fd = open(exelog, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); 
    if (fd < 0) err_exit("open(exelog) fail\n");
    dup2(fd, 1);
    dup2(fd, 2);
    close(fd);

    execvp(cmd->prog, cmd->args);
  }

  int child_status = 0;
  waitpid(pid, &child_status, 0);
  return WEXITSTATUS(child_status);
}

static int execute_all(void)
{
  debug("\n\n==========================================================\n");
  debug("executing all commands, detailed log in [%s]\n", exelog);
  debug("==========================================================\n");

  command_t *iter;
  for (iter = all_cmds; iter; iter = iter->next) {
    iter->real_ret = execute(iter);
    check_executed_result(iter);
  }
}

static int set_env() {}

int main(int argc, char *argv[])
{
  if (argc != 2) err_exit("./functester /path/to/cmd/file\n");

  const char *cmdfile = argv[1];
  construct_all_cmds(cmdfile);

  execute_all();

  return 0;
}
