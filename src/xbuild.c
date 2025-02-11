#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_PATH_LEN (10240)
#define MAX_ENV_LEN (81920)

static void usage(char *prog) {
    fprintf(stderr, "%s app app_args ...\n",
	    prog);
    return;
}

int main(int argc,char *argv[]) {
    char *appdir = NULL;
    char *top = NULL;
    char mypath[MAX_PATH_LEN];
    char appRun[MAX_PATH_LEN];
    char pathenv[MAX_ENV_LEN];
    char appbasenv[MAX_ENV_LEN];
    char efile[MAX_PATH_LEN];

    if (argc < 2) {
	usage(argv[0]);
	return -1;
    }

    appdir = getenv("APPDIR");
    if (appdir == NULL) {
	fprintf(stderr, "no appdir defined\n");
	return -1;
    }
    
    printf("appdir: %s\n", appdir);
    sprintf(appRun,"%s/AppRun", appdir);
    
    memset(mypath, 0x0, sizeof(mypath));
    readlink(appRun, mypath, sizeof(mypath) - 1);

    appdir = dirname(mypath);
    printf("mypath: %s\n", mypath);
    printf("appdir: %s\n", appdir);
    
    sprintf(pathenv,"PATH=%s:%s",
	    appdir, getenv("PATH"));
    sprintf(appbasenv,"APP_BASE=%s/..",
	    appdir);

    putenv(pathenv);
    putenv(appbasenv);
    
    sprintf(efile,"%s/%s",
	    appdir, argv[1]);
    
    if (execvp(efile, &argv[1]) < 0) {
	perror("execv");
	return -1;
    }
    return 0;
}
