#include <stdlib.h>
#include <string.h>
#include <gc.h>

#include "uimpl.h"

int main(int, struct uuarray *);

/*
  j'ai remplacé le point d'entré normal du programme (main)
  par la fonction ci dessous (__start_main) pour adapter les
  les paramètres avec le format du tableau que j'ai choisi:
 */
int __start_main(int argc, char **argv, char **envp)
{
    struct uuarray *arr = GC_MALLOC((sizeof*argv) * (argc+2));
    memcpy(arr, &argv[-1], (sizeof*argv) * (argc+2));
    return main(argc, arr);
}
