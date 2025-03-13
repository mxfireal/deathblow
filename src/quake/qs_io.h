#include <stdio.h>



int COM_FindFile (const char *filename, int *handle, FILE **file, unsigned int *path_id);


char* QSIO_allocfile(FILE **file,int*fs);
void QSIO_ensuredir(const char* dirname);
void QSIO_formatfilepath(const char* input,char* buffer);