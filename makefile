all: shell pbs

shell: shell.c
	gcc -o shell shell.c fatSupport.c -I.

pbs: pbs.c
	gcc -o pbs pbs.c fatSupport.c -I.

pfe: pfe.c
	gcc -o pfe pfe.c fatSupport.c -I.
