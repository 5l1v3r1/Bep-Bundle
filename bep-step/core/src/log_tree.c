#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/log.h"
#include "../include/log_tree.h"
#include "../include/proc.h"

int init_log(char *log_name){
	int ret; 
	ret = clear_log(log_name);
	if (ret == 0){
		//success
		ret = append_to_log(log_name, "Starting:\n");
	}
	return ret;
}

int clear_log(char *log_name){
	FILE *fp; 

	printf("Warning clearing log file: %s\n", log_name);
	
	fp = fopen(log_name, "w");

	if (fp == NULL){
		printf("Failed to clear the log file\n");
		printf("WARNING %i:\t%s\n", errno, strerror(errno));

		return 1;
	}
	fclose(fp);
	return 0; 
}

int append_to_log(char *log_name, char *data){
	FILE *fp;

	fp = fopen(log_name, "a+");
	if (fp == NULL){
		printf("Failed to write to the log file\n");
		printf("Reason:\t%s\n", strerror(errno));
		return 1;
	}

    fputs(data, fp);
	fclose(fp);
	return 0;
}