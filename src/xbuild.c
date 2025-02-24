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

static char *executable_find(const char *filename) {
    char *path_env = getenv("PATH");
    char *path_copy;
    char *dir;
    char *p = NULL;

    if (path_env == NULL) {
	return NULL;
    }

    path_copy = strdup(path_env);
    if (path_copy == NULL) {
	perror("strdup");
	
	return NULL;
    }

    dir = strtok(path_copy, ":");
    while (dir != NULL) {
	char full_path[40960];
	char *p = NULL;

	snprintf(full_path,sizeof(full_path) - 1, "%s/%s", dir, filename);
	if (access(full_path, X_OK) == 0) {
	    p = strdup(full_path);
	    break;
	}

	dir = strtok(NULL, ":");
    }

    free(path_copy);
    return p;
}

static char *exec_file(char *filename) {
    char *p;

    p = basename(filename);
    if (p == NULL) {
	return NULL;
    }

    return executable_find(p);
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
	char fullpath[PATH_MAX];
	
	if (strchr(argv[0], '/') == NULL) {
	    char *execfilename = exec_file(argv[0]);
	    appdir = dirname(execfilename);
	    free(execfilename);
	
	    realpath(appdir, fullpath);
	}
	else {
	    getcwd(mypath, sizeof(mypath) - 1);
	    snprintf(fullpath, sizeof(fullpath) - 1,
		     "%s/%s", mypath, argv[0]);
	    appdir = dirname(fullpath);
	    realpath(appdir, fullpath);
	}
	
	sprintf(xbuildenv, "XBUILD_DIR=%s",
		fullpath);
	putenv(xbuildenv);
	sprintf(efile, "%s/../libexec/xrun",
		fullpath);
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
