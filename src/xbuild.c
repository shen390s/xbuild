#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH_LEN (10240)
#define MAX_ENV_LEN (81920)

static int debug = 0;

#define DEBUG(fmt, ...)                                                        \
  do {                                                                         \
    if (debug) {                                                               \
      printf(fmt, __VA_ARGS__);                                                \
    }                                                                          \
  } while (0)

#define XSTRING(p) (((p) != NULL? (p):"null"))

static void usage(char *prog) {
  fprintf(stderr, "%s app app_args ...\n", prog);
  return;
}

static char *executable_find(const char *filename) {
  char *path_env = getenv("PATH");
  char *path_copy;
  char *dir;
  char *p = NULL;

  DEBUG("executable_find: filename = %s\n", filename);
  DEBUG("path_env: %s\n", path_env);
  
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

    DEBUG("try dir: %s\n", dir);
    
    snprintf(full_path, sizeof(full_path) - 1, "%s/%s", dir, filename);

    DEBUG("full path: %s\n", full_path);
    
    if (access(full_path, X_OK) == 0) {
      p = strdup(full_path);
      break;
    }

    dir = strtok(NULL, ":");
  }

  DEBUG("found: %s\n", XSTRING(p));
  
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
int main(int argc, char *argv[]) {
  char *appdir = NULL;
  char *dir = NULL;
  char *debugenv = NULL;
  char appRun[MAX_PATH_LEN];
  char mypath[MAX_PATH_LEN];
  char efile[MAX_PATH_LEN];
  char xbuildenv[MAX_PATH_LEN];

  if (argc < 2) {
    usage(argv[0]);
    return -1;
  }

  debugenv = getenv("DEBUG");
  if (debugenv != NULL) {
      if ((strcasecmp(debugenv,"y") == 0) ||
	  (strcasecmp(debugenv, "yes") == 0)) {
	  debug = 1;
      }
  }
  
  appdir = getenv("APPDIR");

  DEBUG("appdir: %s\n", XSTRING(appdir));
  
  if (appdir != NULL) {
    sprintf(appRun, "%s/AppRun", appdir);

    DEBUG("AppRun: %s\n", appRun);
    
    memset(mypath, 0x0, sizeof(mypath));
    readlink(appRun, mypath, sizeof(mypath) - 1);

    dir = dirname(mypath);

    sprintf(xbuildenv, "XBUILD_DIR=%s/%s", appdir, dir);
    putenv(xbuildenv);

    sprintf(efile, "%s/%s/../libexec/xrun", appdir, dir);
  } else {
    char fullpath[PATH_MAX];

    DEBUG("argv[0]: %s\n", argv[0]);
    
    if (strchr(argv[0], '/') == NULL) {
      char *execfilename = exec_file(argv[0]);

      DEBUG("exec filename: %s\n", execfilename);
      
      strcpy(fullpath, execfilename);
      
      free(execfilename);
    } else {
      if (argv[0][0] != '/') {
        getcwd(mypath, sizeof(mypath) - 1);
        snprintf(fullpath, sizeof(fullpath) - 1, "%s/%s", mypath, argv[0]);
      } else {
        snprintf(fullpath, sizeof(fullpath) - 1, "%s", argv[0]);
      }

      DEBUG("fullpath: %s\n", fullpath);
    }

    appdir = dirname(fullpath);

    DEBUG("appdir: %s\n", appdir);
    
    sprintf(xbuildenv, "XBUILD_DIR=%s", appdir);
    putenv(xbuildenv);
    sprintf(efile, "%s/../libexec/xrun", appdir);
  }

  DEBUG("efile: %s\n", efile);
  
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
