#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdio.h>

typedef FILE File;

// Function prototypes for file operations
File* open_file(const char* filename);
void close_file(File* file);
int read_file(File* file, void* buffer, size_t size);
int write_file(File* file, const void* buffer, size_t size);
int load_to_memory(File* file);
void start_kernel();

#endif // FILESYSTEM_H
