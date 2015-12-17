#include <stdlib.h>
#include <stdio.h>
#include <gc.h>

#include "uimpl.h"

int main(int, struct uuarray *);

int __start_main(int argc, char **argv, char **envp)
{
    struct uuarray *arr= GC_MALLOC((sizeof*argv) * (argc+2));
    memcpy(arr, &argv[-1], (sizeof*argv) * (argc+2));
    return main(argc, arr);
}
