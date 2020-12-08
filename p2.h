#include "CHK.h"
#include "getword.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <libgen.h>
#include <dirent.h>

#define MAXITEM 100 //max number of words per line
#define MAXSTORAGE 25500 //100 words can have 255 characters long
#define HEREISDELIM_FILE "stinkydelimitertestfile.bogusextention"
int parse();
void sighandler();
