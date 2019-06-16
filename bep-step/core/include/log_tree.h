#ifndef LOG_TREE_H
#define LOG_TREE_H
#define SIZE 1024
#define LOG_LOC "script-tree.log"

int clear_log(char *log_name);
int append_to_log(char *log_name, char *data);
int init_log(char *log_name);

#endif