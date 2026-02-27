/*
生产者消费者模型
1. 创建两个线程
2. 用锁限制 对array的修改、访问
3. 用 信号量 empty 表示空槽，full 表示填充的槽 
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <semaphore.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int write_index = 0, read_index = 0;
atomic_int run = 1;

sem_t empty, full; 
int array[5] = {0};

// 生产者
void* thread_function1(void *arg)
{
    // pthread_mutex_lock - 阻塞方式
    while (run) {
        sem_wait(&empty); // empty - 1
        pthread_mutex_lock(&mutex);
        // 临界区代码
        int data = rand() % 100;
        array[write_index] = data;
        write_index = (write_index + 1) % 5;  // 循环队列
        // printf("producer = %d\n", data);

        pthread_mutex_unlock(&mutex);
        /*
        如果是 pthread_mutex_trylock 如果锁被占有 立马返回，没被占有返回0
        */
        sem_post(&full); // full + 1
    }

    return NULL;
}

// 消费者
void* thread_function2(void *arg)
{
    while (run) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        // 临界区代码
        int data = array[read_index];
        read_index = (read_index + 1) % 5;
        printf("consumer = %d\n", data);
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
    }

    return NULL;
}

int main()
{
    pthread_t thread_1, thread_2;
    sem_init(&empty, 0, 5);
    sem_init(&full, 0, 0);

    // 创建线程
    if (pthread_create(&thread_1, NULL, thread_function1, NULL)) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&thread_2, NULL, thread_function2, NULL)) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }
    
    sleep(10);
    run = 0;
    sem_post(&empty);  // 踢一脚生产者
    sem_post(&full);   // 踢一脚消费者

    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);

    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    return 0;
}