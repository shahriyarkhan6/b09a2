#include "fdtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* get_fd_type(mode_t mode) {
    switch (mode & S_IFMT) {
        case S_IFREG:  return "REG";   // Regular file
        case S_IFDIR:  return "DIR";   // Directory
        case S_IFCHR:  return "CHR";   // Character device
        case S_IFBLK:  return "BLK";   // Block device
        case S_IFIFO:  return "FIFO";  // FIFO/pipe
        case S_IFLNK:  return "LNK";   // Symbolic link
        case S_IFSOCK: return "SOCK";  // Socket
        default:       return "UNKNOWN";
    }
}

char* get_username(uid_t uid) {
    struct passwd* pw = getpwuid(uid);
    char* name;
    
    if (pw) {
        name = malloc(strlen(pw->pw_name) + 1);
        if (name) {
            strcpy(name, pw->pw_name);
            return name;
        }
    }
    
    name = malloc(8);
    if (name) {
        strcpy(name, "unknown");
    }
    return name;
}

process_info_t* get_process_info(pid_t pid) {
    process_info_t* proc = malloc(sizeof(process_info_t));
    if (!proc) return NULL;

    proc->pid = pid;
    proc->fd_count = 0;
    proc->fds_capacity = 10;
    proc->fds = malloc(proc->fds_capacity * sizeof(fd_info_t));

    char path[4096];
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    FILE* comm_file = fopen(path, "r");
    if (comm_file) {
        if (fgets(proc->comm, sizeof(proc->comm), comm_file)) {
            char* newline = strchr(proc->comm, '\n');
            if (newline) *newline = '\0';
        } else {
            strcpy(proc->comm, "unknown");
        }
        fclose(comm_file);
    } else {
        strcpy(proc->comm, "unknown");
    }

    struct stat st;
    snprintf(path, sizeof(path), "/proc/%d", pid);
    if (stat(path, &st) == 0) {
        proc->uid = st.st_uid;
    } else {
        proc->uid = -1;
    }

    snprintf(path, sizeof(path), "/proc/%d/fd", pid);
    DIR* fd_dir = opendir(path);
    if (!fd_dir) {
        free(proc->fds);
        free(proc);
        return NULL;
    }

    struct dirent* entry;
    while ((entry = readdir(fd_dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        if (proc->fd_count >= proc->fds_capacity) {
            proc->fds_capacity *= 2;
            fd_info_t* new_fds = realloc(proc->fds, proc->fds_capacity * sizeof(fd_info_t));
        if (!new_fds) {
                closedir(fd_dir);
                free(proc->fds);
                free(proc);
                return NULL;
            }
            proc->fds = new_fds;
        }

     fd_info_t* fd = &proc->fds[proc->fd_count];
     fd->fd = atoi(entry->d_name);

        char fd_path[4096];
        snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/%s", pid, entry->d_name);
        ssize_t len = readlink(fd_path, fd->path, sizeof(fd->path) - 1);
        if (len != -1) {
            fd->path[len] = '\0';
        } else {
            strcpy(fd->path, "unknown");
        }

        if (stat(fd_path, &st) == 0) {
            fd->mode = st.st_mode;
            fd->uid = st.st_uid;
            fd->gid = st.st_gid;
            fd->inode = st.st_ino;
            strcpy(fd->type, get_fd_type(st.st_mode));
        }

        proc->fd_count++;
    }

    closedir(fd_dir);
    return proc;
}

void free_process_info(process_info_t* process) {
    if (process) {
        free(process->fds);
    }
} 