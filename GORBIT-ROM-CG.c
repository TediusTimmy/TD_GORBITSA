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
   An exploration into using GCC's Label Pointers for comparison.

   NOTE: GCC is the only compiler that I know of that is capable of compiling this code!

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

#define DISPATCH \
   ++pc; \
   if (pc == (MEM - 1)) goto D; \
   goto *operations[roi[pc]];

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

   void * operations [] =
    {
         &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E,
         &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E,
         &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E,
         &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E,
         &&E, &&A, &&B, &&E, &&D, &&E, &&E, &&G, &&E, &&I, &&E, &&E, &&E, &&E, &&E, &&O,
         &&E, &&E, &&R, &&S, &&T, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E,
         &&E, &&a, &&b, &&E, &&E, &&E, &&E, &&g, &&E, &&i, &&E, &&E, &&E, &&E, &&E, &&o,
         &&E, &&E, &&r, &&s, &&t, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E,
         &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E,
         &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E,
         &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E,
         &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E,
         &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E,
         &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E,
         &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E,
         &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E, &&E
    };

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

   goto *operations[roi[pc]];


G:
   acc = rwd[rod[pc]];

   DISPATCH

O:
   rwd[rod[pc]] = acc;

   DISPATCH

R:
   acc = getchar();

   DISPATCH

B:
   if (0 == acc) pc = rod[pc] - 1;

   DISPATCH

I:
   acc += rod[pc];

   DISPATCH

T:
   putchar(acc);

   DISPATCH

S:
   acc = rod[pc];

   DISPATCH

A:
   acc += rwd[rod[pc]];

   DISPATCH

g:
   acc = rwd[rwd[rod[pc]]];

   DISPATCH

o:
   rwd[rwd[rod[pc]]] = acc;

   DISPATCH

r:
   rwd[rod[pc]] = getchar();

   DISPATCH

b:
   if (0 == acc) pc = rwd[rod[pc]] - 1;

   DISPATCH

i:
   rwd[rod[pc]] += acc;

   DISPATCH

t:
   putchar(rwd[rod[pc]]);

   DISPATCH

s:
   acc ^= rwd[rod[pc]];

   DISPATCH

a:
   acc += rwd[rwd[rod[pc]]];

   DISPATCH


E:
   printf("Attempt to execute illegal instruction at program counter %d. Accumulator: %d. Instruction: %c%d", pc, acc, roi[pc], rod[pc]);
   return 1;

D:
   return 0;
 }
