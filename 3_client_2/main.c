#include<string.h>        // strncpy()
#include<unistd.h>      // sleep()
#include<stdio.h>       // perror(), printf(),
#include<stdlib.h>      // exit()
//#include<sys/types.h>
//#include<sys/ipc.h>
#include<sys/sem.h>       // semget(), semctl() + Macros,
#include<sys/shm.h>       // shmget(), shmat(), shmdt(), shmop()

/* + Подсоединиться к разделяемой области памяти.
 * + Определить количество процессов, подсоединенных к разделяемой области памяти и
 * + записать эту информацию в неё.
 */

#ifndef SEM_ID
#define SEM_ID	10      /* ключ массива семафоров */
#endif
#ifndef SHM_ID
#define SHM_ID	11      /* ключ разделяемой памяти */
#endif
#ifndef PERMS
#define PERMS	0666      /* права доступа */
#endif
#ifndef MAX_STRING
#define MAX_STRING	256
#endif

/* структура сообщения, помещаемого в разделяемую память */
typedef struct{
  int type;
  char string [MAX_STRING];
} message_t;

int main()
{
    int semid, shmid, counter = 0;
    message_t *msg_p;   //адрес сообщения в разделяемой памяти
    struct shmid_ds dsbuf; // информация о РОП
    struct sembuf cbuf = {0, 0, 0}, plus_buf = {0, 1, 0};

    /* получение доступа к массиву семафоров */
    while(((semid = semget (SEM_ID, 1, 0))) < 0 && counter < 5){
        ++counter;
        sleep(1);
    }

    if(counter == 5) {
        printf("I dont want to wait anymore...\nCan not get semaphore\n");
        exit(1);
    }

    counter = 0;
    /* получение доступа к сегменту разделяемой памяти */
    while(((shmid = shmget (SHM_ID, sizeof (message_t), 0))) < 0 && counter < 5){
        ++counter;
        sleep(1);
    }

    if(counter == 5) {
        printf("I dont want to wait anymore...\nCan not get SHM\n");
        exit(1);
    }

    counter = 0;
    /* получение адреса сегмента */
    while((msg_p = (message_t *) shmat (shmid, 0, 0)) == NULL && counter < 5){
        ++counter;
        sleep(1);
    }

    if(counter == 5) {
        printf("I dont want to wait anymore...\nShared memory attach error\n");
        exit(1);
    }

    counter = 0;
    semop(semid, &cbuf, 1);         // проверка на 0
    semop(semid, &plus_buf, 1);     // блокировать (+1)
    shmctl(shmid, IPC_STAT, &dsbuf);

    // Определить количество процессов, подсоединенных к разделяемой области памяти
    sprintf(msg_p->string, "%d", (int)dsbuf.shm_nattch);
    msg_p->type = 1;
    printf("%d\n",(int)dsbuf.shm_nattch);

    plus_buf.sem_op = 2;
    semop(semid, &plus_buf, 1);     //отменить блокировку (+2)
    shmdt (msg_p);                  // отсоединить сегмент разделяемой памяти
    return 0;
}
