#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
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


#include <log.h>
#include <proc.h>

#define MAXBUFFERLEN  999999

/*
	Holds information about the process heirarchy.
*/
struct Node{
	struct Node *parent_node_ptr; 
	struct process *current_proc_ptr;
	struct Node *child_1_ptr, *child_2_ptr, *child_3_ptr, *child_4_ptr;
	struct Node *child_5_ptr, *child_6_ptr, *child_7_ptr, *child_8_ptr;
	struct Node *child_9_ptr, *child_10_ptr, *child_11_ptr, *child_12_ptr;
	struct Node *child_13_ptr, *child_14_ptr, *child_15_ptr, *child_16_ptr;
};


/*
	Structre for holding information about proccess
*/
struct Process{
	pid_t pid, ppid;		
	char exe[MAXBUFFERLEN]; 	
	char cwd[MAXBUFFERLEN];		
	char cmd[MAXBUFFERLEN];
};


/*
	ASCII art for when a new proces has been found. 
*/
void new_proc_msg(void){
 	log_trace("\t\t───────────────────────────────────────────");
 	log_trace("\t\t_, _ __, _  _   __, __,  _,  _, __,  _,  _,");
 	log_trace("\t\t|\\ | |_  |  |   |_) |_) / \\ / ` |_  / ` (_ ");
 	log_trace("\t\t| \\| |   |/\\|   |   | \\ \\ / \\ , |   \\ , , )");
 	log_trace("\t\t~  ~ ~~~ ~  ~   ~   ~ ~  ~   ~  ~~~  ~   ~ ");
 	log_trace("\t\t───────────────────────────────────────────");
	log_trace("");
}


/*
	Creates a new proccess structre. 
*/
struct Process* get_proc_info(int proc_id){
	char str_exe[MAXBUFFERLEN], str_cwd[MAXBUFFERLEN], str_env[MAXBUFFERLEN], str_cmd[MAXBUFFERLEN] = {'\0'} ;
	struct Process current_procces;
	struct Process *proccess_pointer;
	char *ptr_exe = str_exe;
	char *ptr_cwd = str_cwd;
	char *ptr_env = str_env;
	char *ptr_cmd = str_cmd;

	new_proc_msg();
	current_procces.pid = get_pid(proc_id);
	log_info("PID : \t%d", current_procces.pid);

	current_procces.ppid = get_ppid(proc_id);
	log_info("PPID:\t%d", current_procces.ppid);

	if (get_exe(proc_id, ptr_exe) == 0){
		strcpy(current_procces.exe, ptr_exe);
		log_info("EXE : \t%s", ptr_exe);
	}
	else{
		log_error("There was an error fetching \"EXE\" with PID: %d", proc_id);
	}

	if (get_cwd(proc_id, ptr_cwd) == 0){
		strcpy(current_procces.cwd, ptr_cwd);
		log_info("CWD : \t%s", ptr_cwd);
	}
	else{
		log_error("There was an error fetching \"CWD\" with PID: %d", proc_id);
	}

	if (get_cmd(proc_id, ptr_cmd) == 0){
		strcpy(current_procces.cmd, ptr_cmd);
		log_info("CMD : \t%s", ptr_cmd);
	}
	else{
		log_error("There was an error fetching \"CMD\" with PID: %d", proc_id);
	}
	get_fd(proc_id);
	get_env(proc_id, ptr_env);

	proccess_pointer = malloc(sizeof (struct Process));

	return proccess_pointer;
}

// Check if the command that was run is a build script
int check_if_build_script(long pid){
	char buffer[MAXBUFFERLEN];
	char *ptr = buffer;
	get_cmd(pid, ptr);

	if (strcasestr(ptr, "python")){
		return 1;
	}
	if (strcasestr(ptr, "bin/sh")){
		return 1;
	}
	return 0;

}

long search_proc(pid_t pid, long *child_list){
	const char *location = "/proc";
	DIR *d;
	char s[MAXBUFFERLEN] = {'\0'} ;
	char *s_ptr = s;
	struct dirent *dir;
	int ppid;
	int x = 0;
	int current_pid;

	// clear array

	for(x = 0; x < 23; x ++){
		child_list[x] = '\0';
	}

	x = 0;
	d = opendir(location);
	while((dir = readdir(d)) != NULL){
		bzero(s_ptr, MAXBUFFERLEN);
		strncpy(s_ptr, dir->d_name, MAXBUFFERLEN - 1);
		if(s_ptr[0] >= '0' && s_ptr[0] <= '9'){
			current_pid = atoi(s_ptr);
			ppid = get_ppid(current_pid);
			if((int) ppid == (int) pid){
				if (check_if_build_script((long) current_pid) == 1){
					child_list[x] = (long) current_pid;
					x++;
				}
			}
		}
	}
	closedir(d);
	return x;
}

void remove_element(long *array, int index, int array_length){
   int i;
   for(i = index; i < array_length - 1; i++) array[i] = array[i + 1];
}

/*
	Given an inital process (root_pid) this function find's
	new child proccess. 
*/
int find_new_proc(pid_t root_pid){
	struct Process *current_procces; 
	long proc_list[64] = {root_pid};
	long child_list[24];
	long *ptr = proc_list;
	long *child_ptr = child_list;
	int x, y, z, proc_count = 1;
	int found = 0;
	long new_proc; 

	get_proc_info(root_pid);

	/*
		While the build script is running we constantly search for new 
		child procesess who's parents PID originated from a list of known parents. 
	*/

	while(1){
		for(x = 0; x < proc_count; x ++){
			found = 0;
			
			new_proc = search_proc(proc_list[x], child_ptr);
			for(z = 0; z < 23 ;z++){
				if (child_list[z] != '\0'){
					for(y = 0; y < proc_count; y ++){
						if(child_list[z] == proc_list[y]){
							found = 1;
							break;
						}
					}
					if(found == 0){
							proc_list[proc_count] = child_list[z];
							proc_count ++;
							get_proc_info(child_list[z]);
					}
				}				
			}
		}

		for (x = 0; x < proc_count; x++){
			if (check_process_is_running(proc_list[x]) == 1){
				remove_element(ptr, x, 64);
				proc_count--;
			}
		}
	}

	printf("Build script terminiated:)");
	log_info("Build script terminiated:)");
	return 1;
}
