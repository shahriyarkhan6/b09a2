#include "fdtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void display_per_process_table(process_info_t* processes, int count) {
    printf("\n%-8s %-6s %-40s %-10s\n", "PID", "FD", "Filename", "Inode");
    printf("=========================================================\n");
    
    for (int i = 0; i < count; i++) {
        process_info_t* proc = &processes[i];
        printf("\nProcess %d (%s)\n", proc->pid, proc->comm);
        
        for (int j = 0; j < proc->fd_count; j++) {
            fd_info_t* fd = &proc->fds[j];
            printf("%-8d %-6d %-40s %-10lu\n",
                proc->pid,
                fd->fd,
                fd->path,
                fd->inode);
        }
    }
}

void display_system_wide_table(process_info_t* processes, int count) {
    printf("\n%-8s %-6s %-40s %-10s\n", "PID", "FD", "Filename", "Inode");
    printf("=========================================================\n");
    
    for (int i = 0; i < count; i++) {
        process_info_t* proc = &processes[i];
        for (int j = 0; j < proc->fd_count; j++) {
            fd_info_t* fd = &proc->fds[j];
            printf("%-8d %-6d %-40s %-10lu\n",
                proc->pid,
                fd->fd,
                fd->path,
                fd->inode);
        }
    }
}

void display_vnodes_table(process_info_t* processes, int count) {
    printf("\n%-40s %-10s %-6s\n", "Filename", "Inode", "Count");
    printf("=========================================================\n");
    
    for (int i = 0; i < count; i++) {
        process_info_t* proc = &processes[i];
        for (int j = 0; j < proc->fd_count; j++) {
            fd_info_t* fd = &proc->fds[j];
            int vcount = 0;
            
            // Count occurrences of this inode
            for (int k = 0; k < count; k++) {
                process_info_t* other = &processes[k];
                for (int l = 0; l < other->fd_count; l++) {
                    if (other->fds[l].inode == fd->inode) {
                        vcount++;
                    }
                }
            }
            
            printf("%-40s %-10lu %-6d\n",
                fd->path,
                fd->inode,
                vcount);
        }
    }
}

void display_composite_table(process_info_t* processes, int count) {
    printf("\n%-8s %-6s %-40s %-10s\n", "PID", "FD", "Filename", "Inode");
    printf("=================================================\n");
    
    int line = 0;
    for (int i = 0; i < count; i++) {
        process_info_t* proc = &processes[i];
        for (int j = 0; j < proc->fd_count; j++) {
            fd_info_t* fd = &proc->fds[j];
            printf("%-3d %-8d %-3d %-40s %-10lu\n",
                line++,
                proc->pid,
                fd->fd,
                fd->path,
                fd->inode);
        }
    }
    printf("=================================================\n");
}

void display_summary_table(process_info_t* processes, int count) {
    printf("\n     Summary Table\n");
    printf("=============\n");
    
    for (int i = 0; i < count; i++) {
        printf("%d (%d)", processes[i].pid, processes[i].fd_count);
        if (i < count - 1) {
            printf(", ");
        }
    }
    printf(",\n");
}