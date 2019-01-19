#include<stdio.h>       // perror(), printf(),
#include<stdlib.h>      // exit()
//#include<sys/types.h>
//#include<sys/ipc.h>
#include<sys/sem.h>       // semget(), semctl() + Macros,
#include<sys/shm.h>       // shmget(), shmat(), shmdt(), shmctl(), shmop()

/* + Создать набор семафоров и разделяемую область памяти.
 * + Вывести информацию, полученную от клиентов в стандартный файл вывода.
 * + После этого вывести все значения набора семафоров и
 * + удалить РОП и НС.
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
typedef struct
{
  int type;
  char string [MAX_STRING];
} message_t;

union semun{
    int val;
    struct semid_ds *buf;
    ushort *array;
} arg;


int main()
{
      int semid, shmid;
      message_t *msg_p;   //адрес сообщения в разделяемой памяти
      int counter = 0;
      struct sembuf minus2_buf = {0, -2, 0}, minus1_buf = {0, -1, 0};


      /* создание семафора */
      if ((semid = semget (SEM_ID, 1, PERMS | IPC_CREAT)) < 0){
        perror("server: can not create semaphore");
        exit(1);
      }

      /* создание сегмента разделяемой памяти */
      if ((shmid = shmget (SHM_ID, sizeof (message_t), PERMS | IPC_CREAT)) < 0){
        perror("server: can not create shared memory segment");
        exit(1);
      }

      /* подключение сегмента к адресному пространству процесса */
      if ((msg_p = (message_t *) shmat(shmid, 0, 0)) == NULL){
        perror("server: shared memory attach error");
        exit(1);
      }

      arg.val = 0;
      semctl (semid, 0, SETVAL, arg);

      while (counter != 2){
         semop(semid, &minus2_buf, 1);        // установить блокировку (-2)
         if(msg_p->type == 1){
            printf ("%s\n", msg_p->string);
            ++counter;
         }
         semop(semid, &minus1_buf, 1);        // снять блокировку (-1)
      }

      printf ("Значение семафора = %d\n", semctl (semid, 0, GETVAL, 0));

      /* удаление семафора */
      if (semctl (semid, 0, IPC_RMID, 0) < 0)
        perror("server: semaphore remove error");

      /* удаление сегмента разделяемой памяти */
      shmdt (msg_p);
      if (shmctl (shmid, IPC_RMID, 0) < 0)
        perror("server: shared memory remove error");

    return 0;
}
