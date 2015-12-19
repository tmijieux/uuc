#include <stdlib.h>
#include <string.h>
#include <gc.h>

#include "uimpl.h"


int main(int, struct uuarray *, struct uuarray *);

/*
  j'ai remplacé le point d'entré normal du programme (main)
  par la fonction ci dessous (__start_main) pour adapter les
  les paramètres avec le format du tableau que j'ai choisi:
 */
int __start_main(int argc, char **argv, char **envp)
{
    int env_size = 0;
    while (envp[env_size++]);

    struct uuarray *uuargv, *uuenvp;
    
    uuargv = GC_MALLOC((sizeof*argv) * (argc+2));
    memcpy(uuargv, &argv[-1], (sizeof*argv) * (argc+2));

    
    uuenvp = GC_MALLOC((sizeof*envp) * (env_size+2));
    memcpy(uuenvp, &envp[0], (sizeof*argv) * (env_size+1));
    uuenvp->size = (uint64_t) env_size;
    
    return main(argc, uuargv, uuenvp);
}
