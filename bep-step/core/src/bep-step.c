#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include "../include/log.h"
#include "../include/log_tree.h"
#include "../include/proc.h"
#include "../include/find_new_proc.h"

#define MAXBUFFER 999999

sig_atomic_t child_exit_status = 0;

/*
	When the initial build script has died this will
	will clean up the mess and terminate the parent process.
*/
void clean_up_child_process (int signal_number) {
    /* Clean up the child process. */
    int status;
    wait (&status);
    /* Store its exit status in a global variable. */
    child_exit_status = status;
    log_info("child_exit_status %i\n", status);
    exit(0);
}

/*
	Disaply's the useage message. 
*/
int usage(){
	printf("USAGE:\t\t\n");
	printf("./script-tree /bin/sh  build-script.sh build-script-args\n");
	printf("./script-tree /bin/python build-script.py build-script-args\n");
	exit(0);
}

void ascii(){
	log_trace("");
	log_trace("\t██████╗ ███████╗██████╗       ███████╗████████╗███████╗██████╗ ");
	log_trace("\t██╔══██╗██╔════╝██╔══██╗      ██╔════╝╚══██╔══╝██╔════╝██╔══██╗");
	log_trace("\t██████╔╝█████╗  ██████╔╝█████╗███████╗   ██║   █████╗  ██████╔╝");
	log_trace("\t██╔══██╗██╔══╝  ██╔═══╝ ╚════╝╚════██║   ██║   ██╔══╝  ██╔═══╝ ");
	log_trace("\t██████╔╝███████╗██║           ███████║   ██║   ███████╗██║     ");
	log_trace("\t╚═════╝ ╚══════╝╚═╝           ╚══════╝   ╚═╝   ╚══════╝╚═╝     ");
	log_trace("\tIt's not a bug it's a feature, this is my first time writing C.");
	log_trace("-----------------------------------------------------------------");
}

/*
	Setups program and parses arguments
	Fork's two process, one for watching for new processes initiated.
	Another for executing the build script. 
*/
int main(int argc, char* argv[], char* arge[]){
	FILE *log_fd;
	pid_t build_script_pid;
	char build_script_path[SIZE];
	char* cmd_args[argc-1];
	int x, n_cmd_arg = 0;

	// Sort out log file 
  	log_fd = fopen("bep-step.log", "w");
  	log_set_fp(log_fd);
	ascii();

	// validate cmdline 
	if (argc <  3) usage();
	if (access(argv[1], F_OK) == -1){
		printf("WARNING:\t%s does not exist.\n", argv[0]);
		usage();
	}	

	printf("Creating log file at: bep-step.log");

	// Setup a interupt signal for when child dies. 
	// Spawn the new process. 
	signal(SIGCHLD, clean_up_child_process);
	build_script_pid = fork();


	if (build_script_pid == -1){
		printf("WARNING: Failed to create initial proccess\n");
		exit(1);
	}

	// Child process 
	else if (build_script_pid == 0){
		strncpy(build_script_path, argv[1], sizeof(build_script_path));
		for (x = 1; x+1 <= argc; x++){
			cmd_args[x-1] = argv[x];
			n_cmd_arg ++; 
		}
		cmd_args[x] = '\0';
		execve(build_script_path, cmd_args, arge);
		log_warn("WARNING %i:\t%s\n", errno, strerror(errno)); 
		log_warn("There was an error executing the initial build-script.\n");
	}
	// Parent process
	else{

		// From this point only proc.c should be allowed to echo to log 
		// Due to thread saftey. 

		find_new_proc(build_script_pid);
	}
	return 1;
}