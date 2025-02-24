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
    char *dir = NULL;
    char appRun[MAX_PATH_LEN];
    char mypath[MAX_PATH_LEN];
    char efile[MAX_PATH_LEN];
    char xbuildenv[MAX_PATH_LEN];

    if (argc < 2) {
	usage(argv[0]);
	return -1;
    }

    appdir = getenv("APPDIR");
    if (appdir != NULL) {
	sprintf(appRun,"%s/AppRun", appdir);
    
	memset(mypath, 0x0, sizeof(mypath));
	readlink(appRun, mypath, sizeof(mypath) - 1);

	dir = dirname(mypath);
    
	sprintf(xbuildenv,"XBUILD_DIR=%s/%s",
		appdir,dir);
	putenv(xbuildenv);
    
	sprintf(efile,"%s/%s/../libexec/xrun",
		appdir, dir);
    }
    else {
	appdir = realpath(dirname(argv[0]), NULL);
	sprintf(xbuildenv, "XBUILD_DIR=%s/..",
		appdir);
	putenv(xbuildenv);
	sprintf(efile, "%s/../libexec/xrun",
		appdir);
    }

    if (access(efile, X_OK) < 0) {
	perror("access");
	return -1;
    }
    
    argv[0] = efile;
    if (execvp(efile, argv) < 0) {
	perror("execv");
	return -1;
    }
    return 0;
}
