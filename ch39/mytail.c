#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFSIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 3 || argv[1][0] != '-') {
        fprintf(stderr, "usage: mytail -n <file>\n");
        exit(1);
    }
    int n = atoi(argv[1] + 1);

    int fd = open(argv[2], O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    struct stat sb;
    fstat(fd, &sb);
    off_t size = sb.st_size;

    // WALK BACKWARD FROM THE END, ONE BLOCK AT A TIME, COUNTING NEWLINES.
    // A newline at the very last byte just ends the last line, so skip it.
    // After the n-th newline (counting from the end), the last n lines begin.
    char buf[BUFSIZE];
    off_t pos = size;
    off_t start = 0;
    int newlines = 0;
    int done = 0;

    while (pos > 0 && !done) {
        int chunk = pos < BUFSIZE ? pos : BUFSIZE;
        pos -= chunk;
        lseek(fd, pos, SEEK_SET);
        read(fd, buf, chunk);

        for (int i = chunk - 1; i >= 0; i--) {
            if (buf[i] != '\n' || pos + i == size - 1) continue;
            newlines++;
            if (newlines == n) {
                start = pos + i + 1;
                done = 1;
                break;
            }
        }
    }

    // PRINT FROM start TO THE END OF THE FILE.
    lseek(fd, start, SEEK_SET);
    int r;
    while ((r = read(fd, buf, BUFSIZE)) > 0)
        write(STDOUT_FILENO, buf, r);

    close(fd);
    return 0;
}