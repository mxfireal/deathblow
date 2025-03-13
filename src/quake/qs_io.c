#include <stdio.h>
#include <unistd.h>
#include "qs_io.h"
#include <memory.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
// note: unistd doenst exist in windows.. so.. fix that later :)

// needs to be in sync with sys, sorry!
#define MAX_HANDLES 32

extern FILE *sys_handles[];
extern int com_filesize;

void Sys_Error(const char *error, ...);

// quake com findfile
int plain_COM_FindFile(const char *filename, int *handle, FILE **file, unsigned int *path_id);

// my com_findfile
int COM_FindFile(const char *filename, int *handle, FILE **file, unsigned int *path_id)
{
        char pbuffer[512];
        {
                //since this gets used in quake's schtuff, this is backwards compatible.
                //if you need to access something in the root game directory, use ./ (like in LINUX!!)
                //so to access something in /local, you would do ./local/<file>
                int fnstrl = strlen(filename);
                int l_validated = 0;
                for (int i = 0; i < fnstrl; i++)
                {
                        if (filename[i] == '.' && i==0 && fnstrl>2 && filename[1] == '/')
                        {
                                l_validated=1;
                                snprintf(pbuffer,512,"%s",filename + 2);
                                break;
                        }
                }
                if (!l_validated)
                {
                        snprintf(pbuffer, 512, "content/%s", filename);
                }
        }

        int check_exists = !(handle || file);

        if (!access(pbuffer, F_OK) == 0)
        {
                // file didnt exist. so just fallback to quake behaviour

                if (check_exists)
                        return -1;

                return plain_COM_FindFile(filename, handle, file, path_id);
        }

        struct FILE **fptr;
        int closefptr;
        struct FILE *fptr_here;
        if (handle)
        {
                // alright..
                int co = -1;
                for (int i = 0; i < MAX_HANDLES; i++)
                {
                        if (sys_handles[i])
                                continue;
                        co = i;
                }
                if (co == -1)
                {
                        Sys_Error("out of free syshandles :(\n");
                        return;
                }

                sys_handles[co] = fopen(pbuffer, "rb");
                fptr = &sys_handles[co];
                closefptr = 0;
                *handle = co;
        }
        else if (file)
        {
                fptr_here = fopen(pbuffer, "rb");
                fptr = &fptr_here;
                closefptr = 0;
                *file = fptr_here;
        }
        else // for check if file exists
        {
                fptr_here = fopen(pbuffer, "rb");
                fptr = &fptr_here;
                closefptr = 1;
        }

        // alrighty. get the length of the file and then we're done
        fseek(*fptr, 0L, SEEK_END);
        long filesize = ftell(*fptr);
        rewind(*fptr);
        if (closefptr)
        {
                fclose(*fptr);
        }
        com_filesize = filesize;
        return filesize;
}

char *QSIO_allocfile(FILE **fptr, int *fs)
{
        fseek(*fptr, 0L, SEEK_END);
        long filesize = ftell(*fptr);
        if (fs)
                *fs = filesize;
        rewind(*fptr);
        char* buf = malloc(filesize + 1);
        size_t bytes_read = fread(buf, 1, filesize, *fptr);
        return buf;
}

void QSIO_ensuredir(const char *dirname)
{
        mkdir(dirname, 0777);
}