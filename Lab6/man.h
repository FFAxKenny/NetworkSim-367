#define NUMHOSTS 20

typedef struct{ /* Connection used by the manager to a host */ 
   int toHost[NUMHOSTS]; /* Pipe link to host */
   int fromHost[NUMHOSTS]; /*  Pipe link from host */
} managerLink;

typedef struct {
   int numlinks;
   managerLink link[NUMHOSTS];
} manLinkArrayType;

/* 
 * Main loop for the manager.  It repeatedly gets comand from
 * the user and then executes the command
 */
void manMain(manLinkArrayType * manLinkArray);


