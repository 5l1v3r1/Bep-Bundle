#ifndef PROC_H
#define PROC_H
#define SIZE 1024
#define LOG_LOC "script-tree.log"

int get_pid();
int get_ppid();
int get_exe(int proc_id, char *s);
int get_cwd(int proc_id, char *s);
int get_env(int proc_id, char *s);
int get_cmd(int proc_id, char *s);
int get_fd(int proc_id);
int check_process_is_running(int pid);

#endif