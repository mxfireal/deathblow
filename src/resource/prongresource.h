#ifndef PRONG_RESOURCE_H
#define PRONG_RESOURCE_H

//lookup a resource by name
char* PrRs_Lookup(const char* name);
char* PrRs_Lookup_b(const char* name,char** buffer);


//Settings

#define PR_MAX_ACTIVE_FILES 4
#define PR_SEARCHROOT "content"
#endif