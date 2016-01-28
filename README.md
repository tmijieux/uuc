# COMPILE
run:
	make

then:
	./compile list_of_file


# Troubleshooting:
## libgc is lacking:

>if you lack the libgc library you can either fetch it with 
your package manager or on the [boehm-gc official site]
(http://www.hboehm.info/gc/).
When it is built, you can put the shared object under the
'src/c/' directory (where the source files are)
(for recent versions you may need to checkout libatomic\_ops too
under the libatomic\_ops directory in the gc source tree, details
on the gc website)

#### OR
pull the library from my home directory:
it is compiled on the enseirb's student environement
(fedora 64bit)

CFLAGS+=-I ~tmijieux/public/include/
LDFLAGS+=-L ~tmijieux/public/lib/

#### 
NB: the boehm-garbage collector is mandatory for the target language as soon as it
uses any array


#### OR

>desactivate it by setting the NOGC environement variable to some value and
rebuild it. (this desactivate for compilation only, it's still needed in target language)
>**NOGC="anythingButEmptyString" make -B**

