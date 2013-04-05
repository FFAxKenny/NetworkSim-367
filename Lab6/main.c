#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "utilities.h"
#include "link.h"
#include "man.h"
#include "host.h"
#include "net.h"
#include "switch.h"

#define EMPTY_ADDR  0xffff  /* Indicates that the empty address */
                             /* It also indicates that the broadcast address */
#define MAXBUFFER 4000
#define PIPEWRITE 1 
#define PIPEREAD  0

int hostcount = 0;
int linkcount = 0;
int switcount = 0;

void main()
{
	hostState hstate;             /* The host's state */
	switchState sstate;
	linkArrayType linkArray;
	manLinkArrayType manLinkArray;
        char filename[200];
	// Clear Screen
	system("clear");	

	// Get Filename
        printf("Enter Topology ('Star' or 'Tree'): ");
        scanf("%s",filename);

	FILE *file = fopen(filename,"r");
	if(file==NULL)
	{
		printf("\nFile '%s' not found or can't be opened\n Terminating.\n\n",filename);
		return;
	}	
	
	pid_t pid;  /* Process id */
	int physid; /* Physical ID of host */
	int i;
	int j;
	int k;

	// Read in first line of data file to set numbers of LH
	if (file != NULL) {
		char readin[10];
		fscanf(file,"%d %d", &switcount, &hostcount);
		linkcount = (hostcount + switcount - 1)*2;
//		printf("%d %d %d\n", switcount, hostcount, linkcount);		
	}
	else perror("Did not load DataFile"); // kick if no data file loading in
	fclose(file);
	/* 
	 * Create nonblocking (pipes) between manager and hosts 
	 * assuming that hosts have physical IDs 0, 1, ... 
	 */
	manLinkArray.numlinks = hostcount+switcount;
	netCreateConnections(& manLinkArray);

	/* Create links between nodes but not setting their end nodes */

	linkArray.numlinks = linkcount;
	netCreateLinks(& linkArray);
	
	/* Set the end nodes of the links */
	netSetNetworkTopology(& linkArray, filename);

	/* Create nodes and spawn their own processes, one process per node */ 

//	printf("Switchnum: %d, Hostnum: %d\n\n",switcount,hostcount);
//	printf("Entering Host And Switch Construction Loop\n");
	for (physid = 0; physid < hostcount+switcount; physid++)
	{
		//printf("Creating Physical ID: %d\n", physid); 
		pid = fork();
	
		if (pid == -1)
		{
			printf("Error:  the fork() failed\n");
			return;
		}
	
		/* The Child Process -- A Host Node */
		else if (pid == 0)
		{ 
	
			// MAIN SWITCH CREATION
			if (physid < switcount)
			{
				printf("Creating Switch With Physical ID: %d\n", physid);
				// Switches after hosts
				switchInit(&sstate, physid);
				// ADD MORE
	
	     			/* Initialize the switch's incident communication links */
				
				/* Switch's outgoing link */
				
				k = netHostOutLink(&linkArray, physid);
				sstate.linkout = linkArray.link[k];
//				printf("Linkout k(%d) For Switch With PhysID(%d)\n",k,physid);
				k = netHostInLink(&linkArray, physid);
				sstate.linkin = linkArray.link[k];
//				printf("Linkin k(%d) For Switch With PhysID(%d)\n",k,physid);
	
				/* Close all other links -- not connected to the switch */
				netCloseHostOtherLinks(& linkArray, physid);
			
			
//				printf("\nLaunching Switch with PhysID: %d\n",physid);
				switchMain(&sstate,&linkArray,filename);
			}
	
			// MAIN HOST CREATION
			else if (physid < switcount+hostcount)
			{
				printf("Creating Host With Physical ID: %d\n", physid);
				hostInit(&hstate, physid); /* Initialize host's state */
				
				/* Initialize the connection to the manager */ 
				hstate.manLink = manLinkArray.link[physid];
	
				/* 
				 * Close all connections not connect to the host
				 * Also close the manager's side of connections to host
				 */
				netCloseConnections(& manLinkArray, physid);
	
				/* Initialize the host's incident communication links */
	
				/* Host's outgoing link*/
				k = netHostOutLink(&linkArray, physid);
				hstate.linkout = linkArray.link[k];
//				printf("Linkout is link %d For Switch With PhysID(%d)\n",k,physid);
	
				/* Host's incoming link */
				k = netHostInLink(&linkArray, physid);
				hstate.linkin = linkArray.link[k];
//				printf("Linkin is link %d For Switch With PhysID(%d)\n",k,physid);
	
				/* Close all other links -- not connected to the host */
				netCloseHostOtherLinks(& linkArray, physid);
	
//				printf("\nLaunching Host with PhysID: %d\n",physid);
	
				/* Go to the main loop of the host node */
				hostMain(&hstate);
	
			}

		}

	}

	/* Manager */

	/* 
	 * The manager is connected to the hosts and doesn't
	 * need the links between nodes
	 */

	/* Close all links between nodes */
	netCloseLinks(&linkArray);

	/* Close the host's side of connections between a host and manager */
	netCloseManConnections(&manLinkArray);

	/* Wait 2 seconds */
	usleep(10000);

	printf("\nLaunching Manager\n\n");
	/* Go to main loop for the manager */
	manMain(& manLinkArray);

	/* 
	 * We reach here if the user types the "q" (quit) command.
	 * Now if we don't do anything, the child processes will continue even
	 * after we terminate the parent process.  That's because these
	 * child proceses are running an infinite loop and do not exit 
	 * properly.  Since they have no parent, and no way of controlling
	 * them, they are called "zombie" processes.  Actually, to get rid
	 * of them you would list your processes using the LINUX command
	 * "ps -x".  Then kill them one by one using the "kill" command.  
	 * To use the kill the command just type "kill" and the process ID (PID).
	 *
	 * The following system call will kill all the children processes, so
	 * that saves us some manual labor
	 */
	kill(0, SIGKILL); /* Kill all processes */
}




