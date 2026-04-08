#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>

#define MYDEV_IOC_MAGIC   'M'
#define MYDEV_CLEAR       _IO(MYDEV_IOC_MAGIC, 0)
#define MYDEV_GET_LEN     _IOR(MYDEV_IOC_MAGIC, 1, int)
#define MYDEV_SET_MODE    _IOW(MYDEV_IOC_MAGIC, 2, int)
#define MYDEV_GET_MODE    _IOR(MYDEV_IOC_MAGIC, 3, int)

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

    // 测试 ioctl 功能，获取当前数据长度
    ioctl(fd, MYDEV_GET_LEN, &ret);
    printf("Current data length: %zd\n", ret);

    // 测试 ioctl 功能，清空数据
    ioctl(fd, MYDEV_CLEAR);
    printf("Data cleared\n");

    // 测试 ioctl功能，设置模式
    int mode = 1;
    ioctl(fd, MYDEV_SET_MODE, &mode);


    ioctl(fd, MYDEV_GET_MODE, &mode);
    printf("Current mode: %d\n", mode);

    close(fd);
    return EXIT_SUCCESS;
}