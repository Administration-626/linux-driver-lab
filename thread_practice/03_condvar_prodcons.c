/*
使用条件变量实现生产者消费者模型
产生偶数使用条件变量通知消费者，消费者只消费偶数
使用互斥锁保护共享资源，条件变量实现线程间的同步
需要判断队列是否满或空，避免死锁和资源浪费
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdatomic.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int write_index = 0, read_index = 0;
atomic_int run = 1;

pthread_cond_t cond  = PTHREAD_COND_INITIALIZER;
int array[5] = {0};

// 生产者
void* producer(void *arg)
{
    // pthread_mutex_lock - 阻塞方式
    int debug_count = 0;
    int flag = 0;
    while (run && debug_count++ < 50) {
        usleep(100000);  // 0.1秒
        pthread_mutex_lock(&mutex);
        printf("[P] before lock, w=%d r=%d\n", write_index, read_index);
        // 判满
        if ((write_index + 1) % 5 == read_index) {
            pthread_mutex_unlock(&mutex); // 释放锁，等待消费者消费
            printf("[P] FULL!\n");  // 看是不是真满了
            continue; // 继续下一轮循环
        }

        // printf("%s %d\n", __func__, write_index);
        // 临界区代码
        int data = rand() % 100;
        array[write_index] = data;
        write_index = (write_index + 1) % 5;  // 循环队列
        printf("producer = %d\n", data);
        if (data % 2 == 0) {
            pthread_cond_signal(&cond); // 生产者发信号，唤醒消费者
            /*
            这里如果没有pthread_cond_wait 的线程在等待条件变量，pthread_cond_signal 发信号会丢失，消费者就无法被唤醒
            */
        } else {
            flag ++;
            if(flag == 5) {
                run = 0; // 生产5个奇数后退出，测试消费者等待偶数的情况
                /* 这里不退出 也会full 的*/
            }
        }
        
        pthread_mutex_unlock(&mutex);
        /*
        如果是 pthread_mutex_trylock 如果锁被占有 立马返回，没被占有返回0
        */
    }

    return NULL;
}

// 消费者
void* consumer(void *arg)
{
    while (run) {
        usleep(100000);  // 0.1秒
        pthread_mutex_lock(&mutex);
        while (1) {
            // 从read开始找到write之间的偶数
            int temp_idx = read_index;
            int found = 0;
            while (temp_idx != write_index) {
                if (array[temp_idx] % 2 == 0) {
                    // 找到了！
                    int data = array[temp_idx];
                    read_index = (temp_idx + 1) % 5;
                    printf("consumer = %d\n", data);
                    found = 1;
                    break;
                }
                temp_idx = (temp_idx + 1) % 5;
            }
            if (found) {
                break; // 继续下一轮消费
            }
            pthread_cond_wait(&cond, &mutex); // 等待条件变量，释放锁并进入睡眠状态
        }
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main()
{
    pthread_t thread_1, thread_2;

    // 创建线程
    if (pthread_create(&thread_1, NULL, producer, NULL)) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&thread_2, NULL, consumer, NULL)) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }
    
    sleep(10);
    run = 0;

    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);

    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);

    return 0;
}