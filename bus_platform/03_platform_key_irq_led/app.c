#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#define DEVICE_PATH "/dev/chrdev_mytrigger"
int main(int argc, char *argv[])
{
    int fd;
    char buf[128];

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    // 测试写操作
    char write_data = '1'; // 默认写入 "1" 来点亮 LED
    if (argc > 1) {
        write_data = atoi(argv[1]) ?
            '1' :
            '0'; // 如果提供了参数，根据参数值决定写入 "1" 还是 "0"
    } else {
        printf("No argument provided, defaulting to write '1' to turn on the LED.\n");
    }

    ssize_t bytes_written = write(fd, &write_data, sizeof(write_data));
    if (bytes_written < 0) {
        perror("Failed to write to device");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Written to device: %c\n", write_data);

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