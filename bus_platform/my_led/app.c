#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEVICE_PATH "/dev/chrdev_myled"
int main()
{
    int fd;
    char buf[128];

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    // 测试写操作
    const char *write_data = "Hello, LED!";
    ssize_t bytes_written = write(fd, write_data, strlen(write_data));
    if (bytes_written < 0) {
        perror("Failed to write to device");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Written to device: %s\n", write_data);

    // 测试读操作
    ssize_t bytes_read = read(fd, buf, sizeof(buf) - 1);
    if (bytes_read < 0) {
        perror("Failed to read from device");
        close(fd);
        return EXIT_FAILURE;
    }
    buf[bytes_read] = '\0'; // Null-terminate the buffer
    printf("Read from device: %s\n", buf);

    close(fd);
    return EXIT_SUCCESS;
}