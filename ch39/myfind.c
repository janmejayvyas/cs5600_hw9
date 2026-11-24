#include <dirent.h>
#include <fnmatch.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// Search options (all optional):
//   -name PATTERN   only print entries whose name matches, e.g. "*.c"
//   -type f|d       only print regular files (f) or directories (d)
//   -maxdepth N     don't go more than N levels below the starting point
char *name_pattern = NULL;
char type_filter = 0;
int maxdepth = -1;

int matches(const char *path, struct stat *sb) {
    if (name_pattern) {
        const char *base = strrchr(path, '/');
        base = base ? base + 1 : path;
        if (fnmatch(name_pattern, base, 0) != 0) return 0;
    }
    if (type_filter == 'f' && !S_ISREG(sb->st_mode)) return 0;
    if (type_filter == 'd' && !S_ISDIR(sb->st_mode)) return 0;
    return 1;
}

void walk(const char *path, int depth) {
    struct stat sb;
    if (lstat(path, &sb) != 0) {
        perror(path);
        return;
    }

    if (matches(path, &sb)) printf("%s\n", path);

    // Only directories have children. lstat() reports a symlink to a
    // directory as a link, not a directory, so we never follow links
    // (this avoids looping forever on a link that points back up the tree).
    if (!S_ISDIR(sb.st_mode) || depth == maxdepth) return;

    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror(path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." or we would recurse into ourselves and our parent.
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char child[PATH_MAX];
        snprintf(child, sizeof child, "%s/%s", path, entry->d_name);
        walk(child, depth + 1);
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    char *root = ".";
    int i = 1;

    // An optional first argument that isn't an option is the starting directory.
    if (i < argc && argv[i][0] != '-') root = argv[i++];

    for (; i + 1 < argc; i += 2) {
        if (strcmp(argv[i], "-name") == 0) name_pattern = argv[i + 1];
        else if (strcmp(argv[i], "-type") == 0) type_filter = argv[i + 1][0];
        else if (strcmp(argv[i], "-maxdepth") == 0) maxdepth = atoi(argv[i + 1]);
        else {
            fprintf(stderr, "unknown option: %s\n", argv[i]);
            exit(1);
        }
    }

    walk(root, 0);
    return 0;
}