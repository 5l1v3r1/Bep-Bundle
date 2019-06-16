#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>       // calloc
#include <stdarg.h>       // va_*
#include <string.h>       // strlen, strcpy
#include <dirent.h>		  // ls 
#include "../include/proc.h"
#include "../include/log.h"

#define MAXBUFFERLEN  2000

/*
	This function is used to read files that contain 
	NULL delimetered lines. 
*/
void get_null_embedded_proc_file (char *fname, char *s)
{
	int fd;
	char env_buf[MAXBUFFERLEN];
	char arg_list[1024] = {'\0'};
	char *env_ptr = env_buf;
	size_t length;
	char* next_arg;
	
	fd = open(fname, O_RDONLY);
	length = read(fd, arg_list, sizeof (arg_list));
	close (fd);
	
	/* read does not NUL-terminate the buffer, so do it here. */
	
	arg_list[length] = '\0';
	
	/* Loop over arguments. Arguments are separated by NULs. */
	next_arg = arg_list;
	while (next_arg < arg_list + length) {
	
		/* Print the argument. Each is NUL-terminated, so just treat it
		like an ordinary string.

		Advance to the next argument. Since each argument is
		NUL-terminated, strlen counts the length of the next argument,
		not the entire argument list. */
		
		log_info("ENV : \t%s", next_arg);
		next_arg += strlen(next_arg) + 1;
	}

	strcpy(s, env_ptr);
}

/*
	Takes an arbitray number of parameters
	Join's them all together as a string. 
*/
char* concat(int count, ...)
{
    va_list ap;
    int i;

    // Find required length to store merged string
    int len = 1; // room for NULL
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
        len += strlen(va_arg(ap, char*));
    va_end(ap);

    // Allocate memory to concat strings
    char *merged = calloc(sizeof(char),len);
    int null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
    {
        char *s = va_arg(ap, char*);
        strcpy(merged+null_pos, s);
        null_pos += strlen(s);
    }
    va_end(ap);

    return merged;
}

/*
	Function for creating a proc file name to be accessed. 
*/
char* get_file_location(int pid, const char* fname){
	char proc_str[SIZE];
	char* file_location;

 	sprintf(proc_str, "%d", pid);
 	file_location = concat(3, "/proc/", proc_str, fname);
	return file_location;
}

int get_file_contents(const char* f_loc, char *s){
	FILE *fp; 

	fp = fopen(f_loc, "rb");
	if( fp != NULL){
		while (!feof(fp)){
			fread(s, 1, MAXBUFFERLEN, fp);
		}
		fclose(fp);
		return 0;
	}
	else{
		//log_error("Failed to  open the file: %s", f_loc);
		//log_error("%s", strerror(errno));
		return 1;
	}
}

int check_process_is_running(int pid){
	char *file_location;
 	char buffer[MAXBUFFERLEN];
 	char *buffer_ptr = buffer;

	file_location = get_file_location(pid, "/cmdline");
	return get_file_contents(file_location, buffer_ptr);
}

int get_pid(int proc_id){
	return proc_id;
}

/*
	Given a process ID this function find's it's parents
	PID. 
*/
int get_ppid(int proc_id){
 	int element_count = 0, x = 0 , pid_count = 0, ret = 0; 
 	char ppid[10];
 	char buffer[MAXBUFFERLEN];
 	char *file_location; 
 	char *buffer_ptr = buffer;

	file_location = get_file_location(proc_id, "/stat");
 	//proc/id/stat: PID COM S PPID 
	ret = get_file_contents(file_location, buffer_ptr);
	

	if(ret == 0){
		while(buffer[x] != '\0'){
			if (buffer[x] == ' '){
				element_count++;
				if (element_count == 4){
					ppid[pid_count] = '\0';
					break;
				}
			}
			if ((element_count == 3) && (buffer[x] != ' ')){
				ppid[pid_count] = buffer[x];
				pid_count++; 
			}
			x ++;
		}
		if (pid_count == 0){
			//log_error("Failed to read and extract PPID from %s", file_location);
			//log_error("File contained the following:\t%s", buffer);
			return 1;
		}
		return atoi(ppid);
	}
	else{
		//log_error("Failed to open:\t%d", file_location);
		return 1; 
	}
 	return 0;
}

/*
	read's proc/[pid]/exe
*/
int get_exe(int proc_id, char *s){
	char *file_location;
	file_location = get_file_location(proc_id, "/exe");
	readlink(file_location, s, MAXBUFFERLEN);
	return 0;
}

/*
	read's proc/[pid]/cwd
*/
int get_cwd(int proc_id, char* s){
	char *file_location;
	file_location = get_file_location(proc_id, "/cwd");
	readlink(file_location, s, MAXBUFFERLEN);
	return 0;
}

/*
	read's proc/[pid]/environ
*/
int get_env(int proc_id, char* s){
	char *file_location;
	file_location = get_file_location(proc_id, "/environ");
	get_file_contents(file_location, s);
	get_null_embedded_proc_file(file_location, s);

	if (s[0] == '\0') {
		get_file_contents(file_location, s);
	}
	return 0;
}

/*
	read's proc/[pid]/cmdline
*/
int get_cmd(int proc_id, char* s){
	char *file_location;
	file_location = get_file_location(proc_id, "/cmdline");
	return get_file_contents(file_location, s);
} 
/*
	Read's proc/[pid]/stat/[fd]
	Find's all currently open file descriptors owned by a process pid. 
	Then follows thw file desriptor to it's absolute path. 
*/
int get_fd(int proc_id){
	// Find's fd's open baseed of process ID

	char *file_location;
	file_location = get_file_location(proc_id, "/fd");
	printf("%s\n", file_location);
	DIR *d;

	const char *bad = ".";
	const char *bad2 = "..";
	const char *bad3 = "/dev/";

	char s[MAXBUFFERLEN] = {'\0'};
	char *s_ptr = s;
	struct dirent *dir;
	char dev_test[6];
	char *dev_ptr = dev_test;

	dev_test[6] = '\0';

	d = opendir(file_location);
	if(d){
		while((dir = readdir(d)) != NULL){
			//file_location = concat(3, "/proc/", proc_str, fname);
			bzero(s_ptr, MAXBUFFERLEN);
			readlink(concat(3, file_location, "/", dir->d_name), s_ptr, MAXBUFFERLEN);
			if ((strcmp(s_ptr, bad) != 0 ) && (strcmp(s_ptr, bad2) != 0)){
				if(s[0] != '\0'){
					strncpy(dev_test, s_ptr, sizeof(dev_test) - 1);
					if ((strcmp(bad3, dev_ptr) != 0)){
						log_info("FD  :\t%s", s_ptr);
					}
				}

			}
		}
		return 1;
	}
	else{
		log_error("Could not open %s", file_location);
		return 0;
	}
}