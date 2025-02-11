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
    char efile[MAX_PATH_LEN];

    if (argc < 2) {
	usage(argv[0]);
	return -1;
    }

    printf("argv[0]:%s\n ", argv[0]);
    
    dirname(realpath(argv[0], mypath));
    appdir = mypath;
    printf("appdir: %s\n", appdir);
    
    sprintf(pathenv,"PATH=%s:%s",
	    appdir, getenv("PATH"));
    sprintf(appbasenv,"APP_BASE=%s/..",
	    appdir);

    putenv(pathenv);
    putenv(appbasenv);
    
    sprintf(efile,"%s/%s",
	    appdir, argv[1]);
    printf("I will exec %s\n", efile);
    
    if (execvp(efile, &argv[1]) < 0) {
	perror("execv");
	return -1;
    }
    return 0;
}
