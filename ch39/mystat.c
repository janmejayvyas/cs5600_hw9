#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

// Build the "drwxr-xr-x" style string from the mode bits.
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

const char *file_type(mode_t mode) {
    if (S_ISREG(mode)) return "regular file";
    if (S_ISDIR(mode)) return "directory";
    if (S_ISLNK(mode)) return "symbolic link";
    return "other";
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: mystat <file>\n");
        exit(1);
    }

    struct stat sb;
    if (stat(argv[1], &sb) != 0) {
        perror("stat");
        exit(1);
    }

    char perms[11];
    mode_string(sb.st_mode, perms);

    printf("  File: %s\n", argv[1]);
    printf("  Size: %lld\tBlocks: %lld\tIO Block: %ld\t%s\n",
           (long long) sb.st_size, (long long) sb.st_blocks,
           (long) sb.st_blksize, file_type(sb.st_mode));
    printf(" Inode: %lu\tLinks: %lu\n",
           (unsigned long) sb.st_ino, (unsigned long) sb.st_nlink);
    printf("Access: (%04o/%s)\tUid: %d\tGid: %d\n",
           sb.st_mode & 0777, perms, (int) sb.st_uid, (int) sb.st_gid);
    printf("Access: %s", ctime(&sb.st_atime));
    printf("Modify: %s", ctime(&sb.st_mtime));
    printf("Change: %s", ctime(&sb.st_ctime));
    return 0;
}