all: shell pbs pfe pwd ls cd

shell: shell.c
	gcc -o shell shell.c -I.

pbs: pbs.c
	gcc -o pbs pbs.c -I.

pfe: pfe.c
	gcc -o pfe pfe.c -I.

pwd: pwd.c
	gcc -o pwd pwd.c

ls: ls.c
	gcc -o ls ls.c -I.

cd: cd.c
	gcc -o cd cd.c -I.