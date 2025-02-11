#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <unistd.h>

#define MAX_PATH_LEN (10240)
#define MAX_ENV_LEN (81920)

static void usage(char *prog) {
    fprintf(stderr, "%s app app_args ...\n",
	    prog);
    return;
}

int main(int argc,char *argv[]) {
    char mypath[MAX_PATH_LEN];
    char *appdir;
    char pathenv[MAX_ENV_LEN];
    char appbasenv[MAX_ENV_LEN];

    if (argc < 2) {
	usage(argv[0]);
	return -1;
    }
    appdir = dirname(realpath(argv[0], mypath));
    sprintf(pathenv,"PATH=%s:%s",
	    appdir, getenv("PATH"));
    sprintf(appbasenv,"APP_BASE=%s/..",
	    appdir);

    putenv(pathenv);
    putenv(appbasenv);
    
    printf("I will exec %s\n", argv[1]);
    
    if (execvp(argv[1], &argv[1]) < 0) {
	perror("execv");
	return -1;
    }
    return 0;
}
