/*
Copyright (c) 2020 Thomas DiModica.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of Thomas DiModica nor the names of other contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THOMAS DIMODICA AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THOMAS DIMODICA OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

/*
   An exploration into using a switch for performance comparisons.

   Results, after averaging 10 runs (the standard deviation was less than 1% of the averages):
      Best TCO, all others are normalized to this.
      Non-setjmp/longjmp threaded interpreter -O2, at 1.5 times slower
      Switch -O2, at 1.7 times slower
      Setjmp/longjmp threaded interpreter -O2, 2 times slower
      Switch -O0, 2.2 times slower
      Setjmp/longjmp threaded interpreter -O0, 4 times slower
      Non-setjmp/longjmp threaded interpreter -O0, at 8.5 times slower
*/

#include <stdio.h>
#include <stdlib.h>

/*
G     ACC = MEM[IMM]
O     MEM[IMM] = ACC
R     ACC = INPUT
B     BRZ IMM
I     ACC += IMM
T     PRINT ACC
S     ACC = IMM
A     ACC += MEM[IMM]

g     ACC = MEM[MEM[IMM]]
o     MEM[MEM[IMM]] = ACC
r     MEM[IMM] = INPUT
b     BRZ MEM[IMM]
i     MEM[IMM] += ACC
t     PRINT MEM[IMM]
s     ACC ^= MEM[IMM]
a     ACC += MEM[MEM[IMM]]

Translation:
   ACC      *acc
   IMM      rod[*pc]
   MEM      rwd
*/

#define MEM 256

void loadToMem(unsigned char * roi, unsigned char * rod, FILE* source)
 {
   int input, cur;

   input = fgetc(source);
   cur = 0;

   while (EOF != input)
    {
      roi[cur] = input;
      rod[cur] = 0;

      input = fgetc(source);

      while ((input >= '0') && (input <= '9'))
       {
         rod[cur] = rod[cur] * 10 + (input - '0');
         input = fgetc(source);
       }

      while ((' ' == input) || ('\t' == input) || ('\n' == input) || ('\r' == input))
       {
         input = fgetc(source);
       }

//printf("loaded instruction %c%d\n", roi[cur], rod[cur]);
      ++cur;
      if (MEM == cur)
       {
         printf("error, program too big\n");
         exit(4);
       }
    }

   if (MEM != cur)
    {
      roi[cur] = 'D'; // Pseudo-instruction "done"
    }
 }

int main (int argc, char ** argv)
 {
   unsigned char roi [MEM], rod [MEM], rwd[MEM], acc;
   int pc;
   FILE * infile;

   for (pc = 0; pc < MEM; ++pc)
    {
      rwd[pc] = 0;
    }

   if (2 != argc)
    {
      printf("usage: GORBIT-ROM source_file\n");
      return 2;
    }
   infile = fopen(argv[1], "r");
   if (NULL == infile)
    {
      printf("cannot open input file\n");
      return 3;
    }
   loadToMem(roi, rod, infile);
   fclose(infile);

   pc = 0;
   acc = 0;

   while (pc < MEM - 1)
    {
      switch (roi[pc])
       {
      case 'G':
         acc = rwd[rod[pc]];
         break;
      case 'O':
         rwd[rod[pc]] = acc;
         break;
      case 'R':
         acc = getchar();
         break;
      case 'B':
         if (0 == acc) pc = rod[pc] - 1;
         break;
      case 'I':
         acc += rod[pc];
         break;
      case 'T':
         putchar(acc);
         break;
      case 'S':
         acc = rod[pc];
         break;
      case 'A':
         acc += rwd[rod[pc]];
         break;
      case 'g':
         acc = rwd[rwd[rod[pc]]];
         break;
      case 'o':
         rwd[rwd[rod[pc]]] = acc;
         break;
      case 'r':
         rwd[rod[pc]] = getchar();
         break;
      case 'b':
         if (0 == acc) pc = rwd[rod[pc]] - 1;
         break;
      case 'i':
         rwd[rod[pc]] += acc;
         break;
      case 't':
         putchar(rwd[rod[pc]]);
         break;
      case 's':
         acc ^= rwd[rod[pc]];
         break;
      case 'a':
         acc += rwd[rwd[rod[pc]]];
         break;
      case 'D':
         pc = MEM;
         break;
      default:
         printf("Attempt to execute illegal instruction at program counter %d. Accumulator: %d. Instruction: %c%d", pc, acc, roi[pc], rod[pc]);
         pc = MEM;
         break;
       }

      ++pc;
    }

   return 0;
 }
