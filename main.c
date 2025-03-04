#include "fdtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    int flag = parse_arguments(argc, argv, 0);
    
    process_info_t* processes = NULL;
    int process_count = 0;

    if (flag > 0) {
        processes = malloc(sizeof(process_info_t));
        if (!processes) {
            perror("memory allocation failed");
            return 1;
        }
        process_info_t* proc = get_process_info(flag);
        if (proc != NULL) {
            processes[0] = *proc;
            free(proc);
            process_count = 1;
        } else {
            fprintf(stderr, "unable to get information for PID %d\n", flag);
            free(processes);
            return 1;
        }
    } else {
        DIR* proc_dir = opendir("/proc");
        if (proc_dir == NULL) {
            perror("failed to open /proc");
            return 1;
        }

        struct dirent* entry;
        while ((entry = readdir(proc_dir)) != NULL) {
            int is_pid = 1;
            int i;
            for (i = 0; entry->d_name[i] != '\0'; i++) {
                if (entry->d_name[i] < '0' || entry->d_name[i] > '9') {
                    is_pid = 0;
                    break;
                }
            }
            
            if (is_pid) {
                pid_t pid = atoi(entry->d_name);
                process_info_t* proc = get_process_info(pid);
                if (proc != NULL) {
                    process_info_t* new_processes = realloc(processes, 
                        (process_count + 1) * sizeof(process_info_t));
                    if (!new_processes) {
                        perror("memory allocation failed");
                        free(processes);
                        closedir(proc_dir);
                        return 1;
                    }
                    processes = new_processes;
                    processes[process_count] = *proc;
                    free(proc);
                    process_count++;
                }
            }
        }
        closedir(proc_dir);
    }

    if (process_count == 0) {
        fprintf(stderr, "no process information available\n");
        return 1;
    }

    if (flag == 1) {
        display_per_process_table(processes, process_count);
    }
    else if (flag == 2) {
        display_system_wide_table(processes, process_count);
    }
    else if (flag == 3) {
        display_vnodes_table(processes, process_count);
    }
    else if (flag == 4) {
        display_composite_table(processes, process_count);
    }
    else if (flag == 5) {
        display_summary_table(processes, process_count);
    }
    else {
        // It's a PID
        process_info_t* proc = get_process_info(flag);
        if (proc != NULL) {
            display_composite_table(proc, 1);
            free_process_info(proc);
        }
    }

    int i;
    for (i = 0; i < process_count; i++) {
        free_process_info(&processes[i]);
    }
    free(processes);

    return 0;
} 