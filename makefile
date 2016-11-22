all: clean copydisk shell pbs pfe pwd ls cd rm rmdir cat touch mkdir df

clean:
	rm build/*

copydisk:
	cp src/disk_images/* build/

shell: src/shell.c
	gcc -o build/shell src/shell.c

pbs: src/pbs.c
	gcc -o build/pbs src/pbs.c

pfe: src/pfe.c
	gcc -o build/pfe src/pfe.c

pwd: src/pwd.c
	gcc -o build/pwd src/pwd.c

ls: src/ls.c
	gcc -o build/ls src/ls.c

cd: src/cd.c
	gcc -o build/cd src/cd.c

rm: src/rm.c
	gcc -o build/rm src/rm.c

rmdir: src/rmdir.c
	gcc -o build/rmdir src/rmdir.c

cat: src/cat.c
	gcc -o build/cat src/cat.c

touch: src/touch.c
	gcc -o build/touch src/touch.c

mkdir: src/mkdir.c
	gcc -o build/mkdir src/mkdir.c

df: src/df.c
	gcc -o build/df src/df.c
