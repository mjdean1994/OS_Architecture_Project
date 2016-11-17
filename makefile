all: shell pbs pfe pwd ls cd rm rmdir cat

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

rm: rm.c
	gcc -o rm rm.c -I.

rmdir: rmdir.c
	gcc -o rmdir rmdir.c -I.

cat: cat.c
	gcc -o cat cat.c -I.