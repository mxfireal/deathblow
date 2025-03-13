
// Prong resource system Version 2.7 C source code
// created by jbone (mxfireal) for gcc
// in this version, you wont have to edit prongresource.h EVER again
#include "prongresource.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "prongresource_funcs.h"
#include <unistd.h>

extern char *resource_table[];


#ifndef PR_SEARCHROOT
#define PR_SEARCHROOT ""
#endif

typedef enum
{
        RTYPE_BINARY, //default
        RTYPE_ASCII, //text
} rtype_t;

typedef enum
{
        RLOCATION_EMBEDDED = 1, //was incbin'd into resourcelayout
        RLOCATION_FILESYS = 2,  //entry is a file path
        RLOCATION_EMBEDDED_FILESYS_FALLBACK = 3, //embedded, but search for the file as a fallback
} rloc_t;

// for quick lookup. does not represent an entry in the layout
typedef struct resource_single_s
{
        char *name;
        char *data;
        char* path;
        long file_size;
        rtype_t rtype;
        rloc_t rlocation;
} resource_t;

int prrs_initialized;
int num_resources; // safe for i < num. last index shouldnt be accessed
resource_t* the_resources; // array

//asterisk hell
void prrs_parse_header_args(char**t_rptr,resource_t*thisrs,char*** l_data,char*** l_fp)
{
        int is_arg=0; char minibuf[64]; char* argstart;
        char* rptr = *t_rptr;
        for (int o=0;o<512;o++) //header stuff
        {
                int continueq=1;
                int run_arg_code = 0;
                if (!rptr[o])
                {
                        *t_rptr += o + 1;
                        run_arg_code = is_arg;
                        continueq=0;
                }
                if (rptr[o] == ' ')
                {
                        run_arg_code = is_arg;
                }
                if (run_arg_code)
                {
                        memset(minibuf,0,sizeof(minibuf));
                        int cpym=&rptr[o] - argstart;
                        PR_Assert(cpym < sizeof(minibuf));
                        if (cpym > sizeof(minibuf))
                        {
                                is_arg=0;
                                continue;
                        }

                        memcpy(minibuf,argstart,cpym);
                        goto label_doop;
                }
                if (rptr[o] == '-')
                {
                        argstart= (&rptr[o]) + 1;
                        is_arg=1;
                        continue;
                }
                if (rptr[o] == '+')
                {
                        argstart= (&rptr[o]) ;
                        is_arg=1;
                        continue;
                }
                continue;
                label_doop:
                if (!strcmp(minibuf,"a"))
                {
                        thisrs->rtype=RTYPE_ASCII;
                        *l_data = &thisrs->data;
                        
                }
                if (!strcmp(minibuf,"b"))
                {
                        thisrs->rtype=RTYPE_BINARY;
                        *l_data = &thisrs->data;
                        
                }
                if (!strcmp(minibuf,"e"))
                {
                        thisrs->rlocation=RLOCATION_FILESYS;
                        *l_fp = &thisrs->path;
                        
                }
                if (!strcmp(minibuf,"F"))
                {
                        thisrs->rlocation=RLOCATION_EMBEDDED_FILESYS_FALLBACK;
                        *l_fp = &thisrs->path;
                        *l_data = &thisrs->data;
                        
                }
                if (continueq)
                continue;
                break;
                
        }
}

void prRs_intialize()
{

        num_resources = 0;
        the_resources = 0;
        for (int i = 0; 1; i++)
        {
                if (resource_table[i]) continue;
                num_resources = i-1;
                break;
        }
        the_resources = malloc(sizeof(resource_t) * (num_resources));

        for (int i =0;i<num_resources;i++)
        {
                char* rptr = resource_table[i];
                char* rptr_next = resource_table[i+1];
                resource_t* thisrs = &the_resources[i];

                memset(thisrs,0,sizeof(resource_t));
                thisrs->rlocation = RLOCATION_EMBEDDED;
                thisrs->file_size = -1; //this is okay

                char** l_fp =0;
                char** l_data = 0;
                prrs_parse_header_args(&rptr,thisrs,&l_data,&l_fp);
                
                //rptr should now be at the name
                thisrs->name = rptr;
                rptr += strlen(rptr)+1;

                if (l_fp)
                {
                        //rptr should now be at the file path name
                        *l_fp = rptr;
                        rptr += strlen(rptr)+1;
                }
                if (l_data)
                {
                        //rptr should now be at the data / filepath
                        *l_data = rptr;
                        rptr += strlen(rptr)+1;
                        thisrs->file_size = rptr_next - rptr;
                }

                //.. thats it!

        }

        prrs_initialized = 1;
}

struct fileref_s
{
        char* data;
};
struct fileref_s pr_filerefs[PR_MAX_ACTIVE_FILES];
static int pr_fref=0;

struct fileref_s* get_first_free_fr()
{
        pr_fref++;
        if (pr_fref >= PR_MAX_ACTIVE_FILES) pr_fref=0;
        if (pr_filerefs[pr_fref].data)
        {
                free(pr_filerefs[pr_fref].data);
        }
        return &pr_filerefs[pr_fref];

}


char* prps_read_file(char* path,resource_t* resource,char** buffer)
{
        if (access(path,F_OK)) return 0;
        FILE* file = fopen(path,"rb");
        if (!file) return 0;
        int psize = 0;
        fseek(file, 0, SEEK_END);
        long filesize = ftell(file);
        rewind(file);
        long count_filesize = filesize;

        if (resource->rtype == RTYPE_ASCII)
        {
                filesize++; //i guess

        }
    
        if (resource)
        {
                resource->file_size = filesize;
        }
        char* storage;
        if (buffer) storage = *buffer;
        else
        {
                char** yeah = &get_first_free_fr()->data;
                *yeah = malloc(filesize);
                storage = *yeah;
        }
        
        size_t bytes_read = fread(storage, 1, count_filesize, file);
        if (bytes_read != count_filesize) {
            fclose(file);
            free(storage);
            *buffer = 0;
            return 0;
        }
        fclose(file);

        if (resource->rtype == RTYPE_ASCII)
        {
                storage[count_filesize]=0; //append a terminator
        }

        return storage;

}


// Public functions

char *PrRs_Lookup_b(const char *name,char** buffer)
{
        if (!prrs_initialized)
                prRs_intialize();

        if (buffer) *buffer = 0;

        for (int i =0;i<num_resources;i++)
        {
                int c = strcmp(the_resources[i].name,name);
                if (c) continue;

                char* path = the_resources[i].path;
                if (path)
                {
                        if (path[0] == '/' || path[0] == '\\') path++;
                        char truepath[128];
                        snprintf(truepath,128,"%s/%s",PR_SEARCHROOT,path);
                        char* d = prps_read_file(truepath,&the_resources[i],buffer);
                        if (d) return d;

                }

                char* data = the_resources[i].data;

                return data ? data : 0;

                
        }
        return 0;
}


char* PrRs_Lookup(const char* name)
{
        return PrRs_Lookup_b(name,0);
}