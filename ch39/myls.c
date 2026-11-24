#include <dirent.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

void mode_string(mode_t mode, char *out) {
    out[0] = S_ISDIR(mode) ? 'd' : S_ISLNK(mode) ? 'l' : '-';
    out[1] = (mode & S_IRUSR) ? 'r' : '-';
    out[2] = (mode & S_IWUSR) ? 'w' : '-';
    out[3] = (mode & S_IXUSR) ? 'x' : '-';
    out[4] = (mode & S_IRGRP) ? 'r' : '-';
    out[5] = (mode & S_IWGRP) ? 'w' : '-';
    out[6] = (mode & S_IXGRP) ? 'x' : '-';
    out[7] = (mode & S_IROTH) ? 'r' : '-';
    out[8] = (mode & S_IWOTH) ? 'w' : '-';
    out[9] = (mode & S_IXOTH) ? 'x' : '-';
    out[10] = '\0';
}

// Print one line of "ls -l" output for the file at path.
void print_long(const char *path, const char *name) {
    struct stat sb;
    if (lstat(path, &sb) != 0) {
        perror(path);
        return;
    }

    char perms[11];
    mode_string(sb.st_mode, perms);

    // Turn the numeric owner/group IDs into names.
    struct passwd *pw = getpwuid(sb.st_uid);
    struct group *gr = getgrgid(sb.st_gid);
    char owner[32], group[32];
    if (pw) snprintf(owner, sizeof owner, "%s", pw->pw_name);
    else snprintf(owner, sizeof owner, "%d", (int) sb.st_uid);
    if (gr) snprintf(group, sizeof group, "%s", gr->gr_name);
    else snprintf(group, sizeof group, "%d", (int) sb.st_gid);

    char when[32];
    strftime(when, sizeof when, "%b %e %H:%M", localtime(&sb.st_mtime));

    printf("%s %3lu %-8s %-8s %8lld %s %s\n", perms,
           (unsigned long) sb.st_nlink, owner, group,
           (long long) sb.st_size, when, name);
}

int main(int argc, char *argv[]) {
    int long_format = 0;
    char *dirname = NULL;
    char cwd[PATH_MAX];

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) long_format = 1;
        else dirname = argv[i];
    }

    // No directory given: use the current working directory.
    if (dirname == NULL) {
        getcwd(cwd, sizeof cwd);
        dirname = cwd;
    }

    DIR *dir = opendir(dirname);
    if (dir == NULL) {
        perror(dirname);
        exit(1);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Like the real ls, hide names starting with '.' (including . and ..).
        if (entry->d_name[0] == '.') continue;

        if (long_format) {
            char path[PATH_MAX + 256];
            snprintf(path, sizeof path, "%s/%s", dirname, entry->d_name);
            print_long(path, entry->d_name);
        } else {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
    return 0;
}