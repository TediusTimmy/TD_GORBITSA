/*
   This is such a trivial program that claiming copyright is a travesty.
*/
#include <stdio.h>

int main (void)
 {
   char string[256];

   fgets(string, 256, stdin);
   while (!feof(stdin))
    {
      if ((string[0] >= '0') && (string[0] <= '9'))
       {
         printf("%c%c%c%c ", string[5], string[6], string[7], string[8]);
       }

      fgets(string, 256, stdin);
    }

   return 0;
 }
