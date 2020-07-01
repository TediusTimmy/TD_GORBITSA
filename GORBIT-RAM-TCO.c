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
   This is an implementation of the GORBITSA esoteric programming language / machine specification.
   Its technically a specification for a virtual machine, that is only suitable for learning exercises,
   whose machine language opcodes are human readable.
   However, as language specifications define the abstract machine in which they run, the point is moot.

   The files GORBIT-RAM and GORBIT-RAM-TCO implement the version where the program is stored in RAM.
   The files GORBIT-ROM and GORBIT-ROM-TCO implement the version where the program is stored in ROM.

   I follow the specification as given here, with expanded instructions: https://esolangs.org/wiki/GORBITSA
   As I am a brainfuck interpreter writer, I will note the machine specific things here:
      IO is character only.
      Output does not print a newline after every character.
      Execution stops at an invalid instruction.
      EOF is -1 (WHY IS THIS NOT IN THE STANDARD SOMEWHERE?)
      Most people will find the behavior of cell size and wrapping to be correct.

   I saw a chance to implement a threaded interpreter, and decided to go for it.

   The theory here is that modern computers are really bad at predicting indirect branches. When you
   select the next instruction with a switch, you have one location in your code with that indirect branch,
   which will almost always be mis-predicted.
   With this threaded code, there is an indirect branch in every instruction. Coupled with common idioms
   in programming, this helps the branch predictor correctly guess which instruction will execute next.

   So, we've changed the problem from "What is the next instruction that will execute?" to "What is the next
   instruction that will execute, given that the last instruction was X?"

   This ought to be about as fast as you can write an interpreter for this, without digressing into more
   archane schemes. And this is probably the fastest way to implement GORBIT-RAM if the code is self-modifying.
   Hopefully the synthetic benchmark will help determine if this is true.

   So, what's with the -TCO versions? That stands for Tail-Call Optimization. GCC 9.3 with -O2 will optimize
   all of these functions to perform proper tail calls: the "call" of the next instruction is actually an
   indirect jump, rather than an indirect call. This means that the stack isn't constantly and wastefully
   growing. In the non-TCO versions, setjmp/longjmp are used to prevent overflowing the stack.

   The thing that you can fault me on is that this program is not good for learning, and it is not good for
   debugging. It's about having an "OMG FAST!!!1" interpreter, without resorting to complicated optimizations.
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
   IMM      rwd[*pc + 1]
   MEM      rwd
*/

#define MEM 256

#define DISPATCH \
   pc += 2; \
   if (pc >= (MEM - 1)) return; \
   operations[rwd[pc]](rwd, pc, acc);

extern void (*operations[])(unsigned char * rwd, int pc, unsigned char acc);

void G (unsigned char * rwd, int pc, unsigned char acc)
 {
   acc = rwd[rwd[pc + 1]];

   DISPATCH
 }

void O (unsigned char * rwd, int pc, unsigned char acc)
 {
   rwd[rwd[pc + 1]] = acc;

   DISPATCH
 }

void R (unsigned char * rwd, int pc, unsigned char acc)
 {
   acc = getchar();

   DISPATCH
 }

void B (unsigned char * rwd, int pc, unsigned char acc)
 {
   if (0 == acc) pc = rwd[pc + 1] - 2;

   DISPATCH
 }

void I (unsigned char * rwd, int pc, unsigned char acc)
 {
   acc += rwd[pc + 1];

   DISPATCH
 }

void T (unsigned char * rwd, int pc, unsigned char acc)
 {
   putchar(acc);

   DISPATCH
 }

void S (unsigned char * rwd, int pc, unsigned char acc)
 {
   acc = rwd[pc + 1];

   DISPATCH
 }

void A (unsigned char * rwd, int pc, unsigned char acc)
 {
   acc += rwd[rwd[pc + 1]];

   DISPATCH
 }

void g (unsigned char * rwd, int pc, unsigned char acc)
 {
   acc = rwd[rwd[rwd[pc + 1]]];

   DISPATCH
 }

void o (unsigned char * rwd, int pc, unsigned char acc)
 {
   rwd[rwd[rwd[pc + 1]]] = acc;

   DISPATCH
 }

void r (unsigned char * rwd, int pc, unsigned char acc)
 {
   rwd[rwd[pc + 1]] = getchar();

   DISPATCH
 }

void b (unsigned char * rwd, int pc, unsigned char acc)
 {
   if (0 == acc) pc = rwd[rwd[pc + 1]] - 2;

   DISPATCH
 }

void i (unsigned char * rwd, int pc, unsigned char acc)
 {
   rwd[rwd[pc + 1]] += acc;

   DISPATCH
 }

void t (unsigned char * rwd, int pc, unsigned char acc)
 {
   putchar(rwd[rwd[pc + 1]]);

   DISPATCH
 }

void s (unsigned char * rwd, int pc, unsigned char acc)
 {
   acc ^= rwd[rwd[pc + 1]];

   DISPATCH
 }

void a (unsigned char * rwd, int pc, unsigned char acc)
 {
   acc += rwd[rwd[rwd[pc + 1]]];

   DISPATCH
 }

void E (unsigned char * rwd, int pc, unsigned char acc)
 {
   printf("Attempt to execute illegal instruction at program counter %d. Accumulator: %d. Instruction: %c%d", pc, acc, rwd[pc], rwd[pc + 1]);
   exit(1);
 }

void D(unsigned char * rwd, int pc, unsigned char acc)
 {
   (void) rwd; (void) pc; (void) acc;
   return;
 }

void (*operations[])(unsigned char * rwd, int pc, unsigned char acc) =
 {
   E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E,
   E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E,
   E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E,
   E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E,
   E, A, B, E, D, E, E, G, E, I, E, E, E, E, E, O,
   E, E, R, S, T, E, E, E, E, E, E, E, E, E, E, E,
   E, a, b, E, E, E, E, g, E, i, E, E, E, E, E, o,
   E, E, r, s, t, E, E, E, E, E, E, E, E, E, E, E,
   E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E,
   E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E,
   E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E,
   E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E,
   E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E,
   E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E,
   E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E,
   E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E
 };

void loadToMem(unsigned char * rwd, FILE* source)
 {
   int input, cur;

   input = fgetc(source);
   cur = 0;

   while (EOF != input)
    {
      rwd[cur] = input;
      rwd[cur + 1] = 0;

      input = fgetc(source);

      while ((input >= '0') && (input <= '9'))
       {
         rwd[cur + 1] = rwd[cur + 1] * 10 + (input - '0');
         input = fgetc(source);
       }

      while ((' ' == input) || ('\t' == input) || ('\n' == input) || ('\r' == input))
       {
         input = fgetc(source);
       }

//printf("loaded instruction %c%d\n", rwd[cur], rwd[cur + 1]);
      cur += 2;
      if (MEM == cur)
       {
         printf("error, program too big\n");
         exit(4);
       }
    }

   if (MEM != cur)
    {
      rwd[cur] = 'D'; // Pseudo-instruction "done"
    }
 }

int main (int argc, char ** argv)
 {
   unsigned char rwd[MEM];
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
   loadToMem(rwd, infile);
   fclose(infile);

   operations[rwd[0]](rwd, 0, 0);

   return 0;
 }
