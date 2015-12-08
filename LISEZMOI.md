# COMPILER
lancer:
	make

puis:
	./compile liste_de_ficher

ou pour avoir juste la sortie en llvm:
avant la moulinette optimisation/assemblage/linkage
      ./uuc fichier  (1 seul fichier)

# Problèmes eventuel:
## pas de garbage collector (libgc):

il est dispo ici: (http://www.hboehm.info/gc/).
OU ici:
dans mon home directory: je l'ai compilé sur l'environnement de l'enseirb
pour les élèves (fedora 64bit), a priori ça devrait marcher sur la plupart
des environnement élèves de l'enseirb

CFLAGS+=-I ~tmijieux/public/include/
LDFLAGS+=-L ~tmijieux/public/lib/

#### 
NB: le gc est nécessaire dans le langage des que l'on utilise des tableaux

pour le compilateur par contre, ce n'est pas obligatoire:
on peut le désactiver dans le compilateur sans trop de soucis:
>** NOGC="pas_la_chaine_vide" make -B **

