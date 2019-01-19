#include<string.h>        // strncpy()
#include<unistd.h>      // sleep()
#include<stdio.h>       // perror(), printf(),
#include<stdlib.h>      // exit()
//#include<sys/types.h>
//#include<sys/ipc.h>
#include<sys/sem.h>       // semget(), semctl() + Macros,
#include<sys/shm.h>       // shmget(), shmat(), shmdt(), shmop()

/* + Подсоединиться к разделяемой области памяти.
 * + Определить подкаталог текущего каталога с максимальным количеством файлов и
 * + записать эту информацию в разделяемую область памяти.
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

int main()
{
      int semid, shmid, counter = 0;
      message_t *msg_p;   // адрес сообщения в разделяемой памяти
      FILE *find_pipe;    // pipe
      char buffer[MAX_STRING];
      struct sembuf cbuf = {0, 0, 0}, plus_buf = {0, 1, 0};
        sleep(15);

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

      semop(semid, &cbuf, 1);         // проверка на 0
      semop(semid, &plus_buf, 1);     // блокировать (+1)

      // Определить подкаталог текущего каталога с максимальным количеством файлов

      // find_pipe = popen("find . -type f| cut -d/ -f2 | sort | uniq -c | sort -nr | head -1 | sed 's/^[ ]*[0-9]*\s//' ", "r");
      find_pipe = popen("find /home/anna/Documents -type f| cut -d/ -f 2-5 | sort | uniq -c | sort -nr | head -1 | sed 's/^[ ]*[0-9]*\\s//'", "r");
      if(!find_pipe || fgets(buffer, MAX_STRING, find_pipe) == NULL) perror("Pipe failed.\n");
      else{
        if(sizeof (buffer) > 0){
            printf("%s",buffer);
            strncpy (msg_p->string, buffer, MAX_STRING);
        }else{
            printf("No catalogs\n");
            strncpy (msg_p->string, "No catalogs", MAX_STRING);
        }
        msg_p->type = 1;
        pclose(find_pipe);
      }
      plus_buf.sem_op = 2;
      semop(semid, &plus_buf, 1);     // отменить блокировку (+2)
      shmdt (msg_p);                  // отсоединить сегмент разделяемой памяти
      return 0;
}
