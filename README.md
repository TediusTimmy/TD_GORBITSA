GORBITSA
========

   This is an implementation of the GORBITSA esoteric programming language / machine specification.
   Its technically a specification for a virtual machine, that is only suitable for learning exercises,
   whose machine language opcodes are human readable.
   However, as language specifications define the abstract machine in which they run, the point is moot.

   The files GORBIT-RAM and GORBIT-RAM-TCO implement the version where the program is stored in RAM.  
   The files GORBIT-ROM and GORBIT-ROM-TCO implement the version where the program is stored in ROM.

   I follow the specification as given here, with expanded instructions: https://esolangs.org/wiki/GORBITSA  
   As I am a brainfuck interpreter writer, I will note the machine specific things here:
*      IO is character only.
*      Output does not print a newline after every character.
*      Execution stops at an invalid instruction.
*      EOF is -1 (WHY IS THIS NOT IN THE STANDARD SOMEWHERE?)
*      Most people will find the behavior of cell size and wrapping to be correct.

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

   Performance Comparison Results, after averaging 10 runs (the standard deviation was less than 1% of the averages):
*      Best TCO, all others are normalized to this.
*      Non-setjmp/longjmp threaded interpreter -O2, at 1.5 times slower
*      Switch -O2, at 1.7 times slower
*      Setjmp/longjmp threaded interpreter -O2, 2 times slower
*      Switch -O0, 2.2 times slower
*      Setjmp/longjmp threaded interpreter -O0, 4 times slower
*      Non-setjmp/longjmp threaded interpreter -O0, at 8.5 times slower

I don't understand that last one, but it was pretty consistent.