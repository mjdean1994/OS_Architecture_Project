/*
Yeah! Come on, come on, come on, come on 
Now touch me, baby 
Can't you see that I am not afraid? 
What was that promise that you made? 
Why won't you tell me what she said? 
What was that promise that you made? 

Now, I'm gonna love you 
Till the heavens stop the rain 
I'm gonna love you 
Till the stars fall from the sky for you and I 

Come on, come on, come on, come on 
Now touch me, baby 
Can't you see that I am not afraid? 
What was that promise that you made? 
Why won't you tell me what she said? 
What was that promise that you made? 

I'm gonna love you 
Till the heavens stop the rain 
I'm gonna love you 
Till the stars fall from the sky for you and I 
I'm gonna love you 
Till the heavens stop the rain 
I'm gonna love you 
Till the stars fall from the sky for you and I 

Stronger than dirt
*/

#include <stdio.h>
#include <stdlib.h>
#include "sharedMemory.h"
#include "utilities.h"

FILE* FILE_SYSTEM_ID;

int main(int argc, char **argv)
{
	//Inspired by https://beej.us/guide/bgipc/output/html/multipage/shm.html
 	int shmId = shmget((key_t) 8675308, sizeof(char), 0666 | IPC_CREAT);
 
 	SharedMemory *sharedMemory = (SharedMemory *)shmat(shmId, (void *) 0, 0);
 
	FILE_SYSTEM_ID = fopen(sharedMemory->floppyImageName, "r+");

	if (FILE_SYSTEM_ID == NULL)
	{
   		printf("Could not open the floppy drive or image.\n");
   		exit(1);
	}

	int count = countFreeClusters();

   printf("Free Clusters: %d\n", count);
}
