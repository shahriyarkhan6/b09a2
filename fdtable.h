#ifndef FDTABLE_H
#define FDTABLE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

// Structure definitions
typedef struct {
    int fd;
    char path[1024];     // Updated size
    char type[16];       // Updated size
    mode_t mode;
    uid_t uid;
    gid_t gid;
    ino_t inode;
} fd_info_t;

typedef struct {
    pid_t pid;
    char comm[128];      // Updated size
    uid_t uid;
    int fd_count;
    fd_info_t* fds;
    int fds_capacity;
} process_info_t;

// Structure to hold program options
typedef struct {
    int per_process;  // Using int instead of bool
    int system_wide;
    int vnodes;
    int composite;
    int summary;
    int threshold;
    pid_t target_pid;
} options_t;

// Function declarations
options_t parse_arguments(int argc, char* argv[]);
process_info_t* get_process_info(pid_t pid);
void display_per_process_table(process_info_t* processes, int count);
void display_system_wide_table(process_info_t* processes, int count);
void display_vnodes_table(process_info_t* processes, int count);
void display_composite_table(process_info_t* processes, int count);
void display_summary_table(process_info_t* processes, int count, int threshold);
void free_process_info(process_info_t* process);
const char* get_fd_type(mode_t mode);
char* get_username(uid_t uid);

#endif