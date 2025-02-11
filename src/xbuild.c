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

char *topdir(char *path) {
    char dir[MAX_PATH_LEN];

    strcpy(dir, path);
    while(1) {
	char filename[MAX_PATH_LEN];
	struct stat sbuf;
	char *p;

	sprintf(filename,"%s/AppRun", dir);
	if (stat(filename, &sbuf) >= 0) {
	    return strdup(dir);
	}

	p = strrchr(dir, '/');
	if (p == NULL) {
	    break;
	}
	*p = 0x0;
    }

    return NULL;
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
	memset(mypath, 0x0, sizeof(mypath));
	readlink("/proc/self/exe",
		 mypath,sizeof(mypath) - 1);
	appdir = dirname(mypath);
    }
    
    top = topdir(appdir);

    if (top == NULL) {
	fprintf(stderr, "can not find top dir\n");
	return -1;
    }
    
    sprintf(appRun,"%s/AppRun", top);
    
    memset(mypath, 0x0, sizeof(mypath));
    readlink(appRun, mypath, sizeof(mypath) - 1);

    appdir = dirname(mypath);
    
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
