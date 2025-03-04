#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

// struct for file descriptor 
typedef struct {
    int fileDescriptor;
    char filePath[1024];
    mode_t fileMode;
    uid_t fileOwnerUID;
    gid_t fileOwnerGID;
    ino_t fileInode;
} FileDescriptorInfo;

// struct for process information
typedef struct {
    pid_t processID;
    char processName[128];
    uid_t processOwnerUID;
    int fdCount;
    FileDescriptorInfo* fdArray;
    int fdArrayCapacity;
} ProcessFileInfo;

// function declarations
void displayProcessFDTable(ProcessFileInfo* processArray, int processCount);
void displaySystemFDTable(ProcessFileInfo* processArray, int processCount);
void displayVnodeFDTable(ProcessFileInfo* processArray, int processCount);
void displayCompositeFDTable(ProcessFileInfo* processArray, int processCount);
void displayFDSummaryTable(ProcessFileInfo* processArray, int processCount);
ProcessFileInfo* getProcessFileInfo(pid_t targetPID);
void freeProcessFileInfo(ProcessFileInfo* processData);

// helper function to convert str to int
int stringToInt(const char* str) {
    int result = 0;
    int i = 0;

    while (str[i] != '\0') {
        if (str[i] < '0' || str[i] > '9') {
            return -1;  
        }
        result = result * 10 + (str[i] - '0');
        i++;
    }
    return result;
}

// getting process information

ProcessFileInfo* getProcessFileInfo(pid_t targetPID) {
    ProcessFileInfo* processInfo = malloc(sizeof(ProcessFileInfo));
    if (!processInfo) {
        return NULL;
    }

    processInfo->processID = targetPID;
    processInfo->fdCount = 0;
    processInfo->fdArrayCapacity = 10;
    processInfo->fdArray = malloc(processInfo->fdArrayCapacity * sizeof(FileDescriptorInfo));
    if (!processInfo->fdArray) {
        free(processInfo);
        return NULL;
    }

    char procPath[4096];
    char pidStr[32];
    struct stat procStat;

    // Build the comm path
    strcpy(procPath, "/proc/");
    sprintf(pidStr, "%d", targetPID);
    strcat(procPath, pidStr);
    strcat(procPath, "/comm");

    FILE* commFile = fopen(procPath, "r");
    if (commFile) {
        if (fgets(processInfo->processName, sizeof(processInfo->processName), commFile)) {
            char* newlineChar = strchr(processInfo->processName, '\n');
            if (newlineChar) {
                *newlineChar = '\0';
            }
        }
        fclose(commFile);
    }

    // Build the proc path
    strcpy(procPath, "/proc/");
    strcat(procPath, pidStr);

    if (stat(procPath, &procStat) == 0) {
        processInfo->processOwnerUID = procStat.st_uid;
    }

    // Build the fd directory path
    strcat(procPath, "/fd");
    DIR* fdDirectory = opendir(procPath);
    if (!fdDirectory) {
        free(processInfo->fdArray);
        free(processInfo);
        return NULL;
    }

    struct dirent* fdEntry;
    while ((fdEntry = readdir(fdDirectory)) != NULL) {
        if (fdEntry->d_name[0] >= '0' && fdEntry->d_name[0] <= '9') {
            if (processInfo->fdCount >= processInfo->fdArrayCapacity) {
                processInfo->fdArrayCapacity *= 2;
                FileDescriptorInfo* newFDArray = realloc(processInfo->fdArray, 
                    processInfo->fdArrayCapacity * sizeof(FileDescriptorInfo));
                if (!newFDArray) {
                    closedir(fdDirectory);
                    free(processInfo->fdArray);
                    free(processInfo);
                    return NULL;
                }
                processInfo->fdArray = newFDArray;
            }

            FileDescriptorInfo* fdInfo = &processInfo->fdArray[processInfo->fdCount];
            fdInfo->fileDescriptor = stringToInt(fdEntry->d_name);

            // Build the fd path
            char fdPath[4096];
            strcpy(fdPath, procPath);
            strcat(fdPath, "/");
            strcat(fdPath, fdEntry->d_name);
            
            // Copy the path directly
            strcpy(fdInfo->filePath, fdPath);

            if (stat(fdPath, &procStat) == 0) {
                fdInfo->fileMode = procStat.st_mode;
                fdInfo->fileOwnerUID = procStat.st_uid;
                fdInfo->fileOwnerGID = procStat.st_gid;
                fdInfo->fileInode = procStat.st_ino;
            }

            processInfo->fdCount++;
        }
    }

    closedir(fdDirectory);
    return processInfo;
}

// displaying summary table
void displayFDSummaryTable(ProcessFileInfo* processArray, int processCount) {
    printf("\n     Summary Table\n");
    printf("==================\n");
    
    for (int processIndex = 0; processIndex < processCount; processIndex++) {
        printf("%d (%d)", processArray[processIndex].processID, 
            processArray[processIndex].fdCount);

        if (processIndex < processCount - 1) {
            printf(", ");
        }
    }
    printf(",\n");
}

// freeing process information
void freeProcessFileInfo(ProcessFileInfo* processData) {
    if (processData) {
        free(processData->fdArray);
    }
}

// displaying composite table
void displayCompositeFDTable(ProcessFileInfo* processArray, int processCount) {
    printf("\n%-8s %-6s %-40s %-10s\n", "PID", "FD", "Filename", "Inode");
    printf("=================================================\n");
    
    for (int processIndex = 0; processIndex < processCount; processIndex++) {

        ProcessFileInfo* currentProcess = &processArray[processIndex];

        for (int fdIndex = 0; fdIndex < currentProcess->fdCount; fdIndex++) {   
            FileDescriptorInfo* currentFD = &currentProcess->fdArray[fdIndex];
            printf("%-8d %-6d %-40s %-10lu\n",
                currentProcess->processID,
                currentFD->fileDescriptor,
                currentFD->filePath,
                currentFD->fileInode);
        }
    }
    printf("=================================================\n");
}

// displaying per-process table
void displayProcessFDTable(ProcessFileInfo* processArray, int processCount) {
    printf("\nProcess FD Table\n");
    printf("===============\n");
    
    for (int processIndex = 0; processIndex < processCount; processIndex++) {
        ProcessFileInfo* currentProcess = &processArray[processIndex];
        printf("\nPID: %d (%s)\n", currentProcess->processID, currentProcess->processName);
        printf("%-6s %-40s %-10s\n", "FD", "Path", "Inode");
        printf("--------------------------------------------------\n");
        
        for (int fdIndex = 0; fdIndex < currentProcess->fdCount; fdIndex++) {
            FileDescriptorInfo* currentFD = &currentProcess->fdArray[fdIndex];
            printf("%-6d %-40s %-10lu\n",
                currentFD->fileDescriptor,
                currentFD->filePath,
                currentFD->fileInode);
        }
    }
}

// displaying system-wide table
void displaySystemFDTable(ProcessFileInfo* processArray, int processCount) {
    printf("\nSystem-wide FD Table\n");
    printf("===================\n");
    printf("%-8s %-6s %-40s\n", "PID", "FD", "Path");
    printf("--------------------------------------------------\n");
    
    for (int processIndex = 0; processIndex < processCount; processIndex++) {
        ProcessFileInfo* currentProcess = &processArray[processIndex];
        for (int fdIndex = 0; fdIndex < currentProcess->fdCount; fdIndex++) {
            FileDescriptorInfo* currentFD = &currentProcess->fdArray[fdIndex];
            printf("%-8d %-6d %-40s\n",
                currentProcess->processID,
                currentFD->fileDescriptor,
                currentFD->filePath);
        }
    }
}

// displaying vnodes table
void displayVnodeFDTable(ProcessFileInfo* processArray, int processCount) {
    printf("\nVnodes FD Table\n");
    printf("===============\n");
    printf("%-10s %-8s %-8s %-8s\n", "Inode", "UID", "GID", "Mode");
    printf("--------------------------------------------------\n");
    
    for (int processIndex = 0; processIndex < processCount; processIndex++) {
        ProcessFileInfo* currentProcess = &processArray[processIndex];
        for (int fdIndex = 0; fdIndex < currentProcess->fdCount; fdIndex++) {
            FileDescriptorInfo* currentFD = &currentProcess->fdArray[fdIndex];
            printf("%-10lu %-8d %-8d %o\n",
                currentFD->fileInode,
                currentFD->fileOwnerUID,
                currentFD->fileOwnerGID,
                currentFD->fileMode);
        }
    }
}

int main(int argc, char* argv[]) {
    char* cmdarg;
    
    if (argc > 1) {
        cmdarg = argv[1];
    } else {
        cmdarg = "--composite";  // defaulting to composite
    }

    ProcessFileInfo* processArray = NULL;
    int processCount = 0;

    DIR* procDirectory = opendir("/proc");
    if (!procDirectory) {
        printf("Can't open /proc\n");
        return 1;
    }

    struct dirent* dirEntry;
    while ((dirEntry = readdir(procDirectory))) {
        if (dirEntry->d_name[0] >= '0' && dirEntry->d_name[0] <= '9') {
            int targetPID = stringToInt(dirEntry->d_name);
            ProcessFileInfo* procInfo = getProcessFileInfo(targetPID);
            if (procInfo) {
                processArray = realloc(processArray, (processCount + 1) * sizeof(ProcessFileInfo));
                processArray[processCount] = *procInfo;
                free(procInfo);
                processCount++;
            }
        }
    }
    closedir(procDirectory);

    // parsing the command line arguments and displaying corresponding table

    if (strcmp(cmdarg, "--per-process") == 0) {
        displayProcessFDTable(processArray, processCount);
    }
    else if (strcmp(cmdarg, "--systemWide") == 0) {
        displaySystemFDTable(processArray, processCount);
    }
    else if (strcmp(cmdarg, "--Vnodes") == 0) {
        displayVnodeFDTable(processArray, processCount);
    }
    else if (strcmp(cmdarg, "--composite") == 0) {
        displayCompositeFDTable(processArray, processCount);
    }
    else if (strcmp(cmdarg, "--summary") == 0) {
        displayFDSummaryTable(processArray, processCount);
    }
    else if (cmdarg[0] != '-') {
        // Must be a PID
        int target_pid = stringToInt(cmdarg);
        if (target_pid != -1) {
            ProcessFileInfo* proc_info = getProcessFileInfo(target_pid);
            if (proc_info) {
                displayCompositeFDTable(proc_info, 1);
                freeProcessFileInfo(proc_info);
                free(proc_info);
            }
        } else {
            printf("Invalid PID format\n");
        }
    }
    else {
        printf("invalid argument: %s\n", cmdarg);
        printf("valid arguments: --per-process, --systemWide, --Vnodes, --composite, or --summary\n");
    }

    // freeing allocated memory
    for (int i = 0; i < processCount; i++) {
        freeProcessFileInfo(&processArray[i]);
    }
    free(processArray);
    return 0;
} 

