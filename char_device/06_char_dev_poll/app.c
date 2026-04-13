#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

int main(void)
{
    int fd = open("/dev/mydev", O_RDWR | O_NONBLOCK);
    struct pollfd pfd = {
        .fd = fd,
        .events = POLLIN,
    };

    while (1) {
        int ret = poll(&pfd, 1, 5000);
        if (ret < 0) {
            perror("poll");
            break;
        } else if (ret == 0) {
            printf("timeout\n");
        } else {
            if (pfd.revents & POLLIN) {
                char buf[16] = {0};
                int n = read(fd, buf, sizeof(buf));
                printf("read %d bytes\n", n);
            }
        }
    }

    close(fd);
    return 0;
}