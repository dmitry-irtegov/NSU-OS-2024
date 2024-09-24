#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
   FILE *file = fopen("file.txt", "r");
   uid_t real_user_id = getuid();
   uid_t effective_id = geteuid();

   printf("real user id: %u\n", real_user_id);
   printf("effective user id: %u\n", effective_id);

   if (file != NULL)
   {
      fclose(file);
   } else
   {
      perror("fopen error when opening file");
   }

   if (setuid(real_user_id) == -1)
   {
      perror("setuid error");
   }
   
   effective_id = geteuid();

   printf("real user id: %u\n", real_user_id);
   printf("effective user id: %u\n", effective_id);

   file = fopen("file.txt", "r");
   if (file != NULL)
   {
      fclose(file);
   } else
   {
      perror("fopen error when opening file");
   }

   return 0;
}