#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gc.h>
#include <sys/time.h>

#include "uimpl.h"

//#define malloc GC_MALLOC
#define malloc malloc

int main(int, struct uuarray *, struct uuarray *);

/*
  j'ai remplacé le point d'entré normal du programme (main)
  par la fonction ci dessous (__start_main) pour adapter les
  les paramètres avec le format du tableau que j'ai choisi:
 */
int __start_main(int argc, char **argv, char **envp)
{
    int env_size = 0, ret, time = 0;
    struct uuarray *uuargv, *uuenvp;
    struct timeval tv1, tv2;
    
    while (envp[env_size++]) {
        if (!strcmp("TIME=true", envp[env_size-1])) {
            time = 1;
            gettimeofday(&tv1, NULL);
        }
    }

    uuargv = malloc((sizeof*argv) * (argc+2));
    memcpy(uuargv, &argv[-1], (sizeof*argv) * (argc+2));
    
    uuenvp = malloc((sizeof*envp) * (env_size+2));
    memcpy(uuenvp, &envp[-1], (sizeof*argv) * (env_size+2));
    uuenvp->size = (uint64_t) env_size;
    
    ret = main(argc, uuargv, uuenvp);

    if (time) {
        gettimeofday(&tv2, NULL);
        printf("time: %lu ms",
               1000000*(tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec));
    }

    return ret;
}
