#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int fd = open("/dev/mychardev", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    char buf[100];
    ssize_t ret = read(fd, buf, sizeof(buf));
    if (ret < 0) {
        perror("Failed to read from device");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Read from device: %.*s\n", (int)ret, buf);

    const char *write_data = "write data 547";
    ret = write(fd, write_data, strlen(write_data));
    if (ret < 0) {
        perror("Failed to write to device");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Wrote to device: %s\n", write_data);

    // 再次读取，看看写入的数据是否成功覆盖了之前的数据
    ret = read(fd, buf, sizeof(buf));
    if (ret < 0) {
        perror("Failed to read from device");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Read from device: %.*s\n", (int)ret, buf);


    close(fd);
    return EXIT_SUCCESS;
}