#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd, ret;
    char buffer[100];

    fd = open("/dev/my_misc_device", O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    ret = read(fd, buffer, sizeof(buffer));
    printf("read returned: %d\n", ret);
    printf("data: %.*s\n", ret, buffer);
    if (ret < 0) {
        perror("read");
        close(fd);
        return -1;
    }

    ret = write(fd, buffer, 20);
    printf("write returned: %d\n", ret);
    printf("wrote data: %.*s\n", ret, buffer);
    if (ret < 0) {
        perror("write");
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}