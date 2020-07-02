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
* IO is character only.
* Output does not print a newline after every character.
* Execution stops at an invalid instruction.
* EOF is -1 (WHY IS THIS NOT IN THE STANDARD SOMEWHERE?)
* Most people will find the behavior of cell size and wrapping to be correct.

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

   (Post note: after doing the benchmarking, using setjmp/longjmp to control the stack size was not as beneficial
   as I had hoped. The results we weird, see below.)

   The thing that you can fault me on is that this program is not good for learning, and it is not good for
   debugging. It's about having an "OMG FAST!!!1" interpreter, without resorting to complicated optimizations.

   Performance Comparison Results:  
   Setup: compute Ackermann(3, 3), with tail-call optimization, 62500.  
      (My implementation has tail-call optimization.)  
      Do this ten times and average the result.  
         Try to get a run of ten where the standard deviation is less than 1% of the average.  
      Normalize the results with respect to the fastest.  

   Results:                                                                               Time factor:
* Tail-call optimized -O2                                                             1
* Tie: non-setjmp/longjmp threaded interpreter -O2, computed goto -O2                 1.5
* Switch -O2                                                                          1.7
* Setjmp/longjmp threaded interpreter -O2                                             2
* Switch -O0                                                                          2.2
* Computed goto -O0                                                                   2.5
* Setjmp/longjmp threaded interpreter -O0                                             4
* Non-setjmp/longjmp threaded interpreter -O0                                         8.5

Finally, I decided to redo this, using -Og instead of -O0.
         This flag is GCCs "debugging safe optimizations" flag.
         This is important in seeing the difference between debugging speed and normal speed.
* Tail-call optimized -O2                                                             1
* Tie: non-setjmp/longjmp threaded interpreter -O2, computed goto -Og and -O2         1.5  
         To two decimals: Computed Goto -O2 1.47, Threaded 1.48, Computed Goto -O0 1.53
* Switch -Og                                                                          1.6
* Switch -O2                                                                          1.7
* Setjmp/longjmp threaded interpreter -O2                                             2
* Setjmp/longjmp threaded interpreter -Og                                             2.6
* Non-setjmp/longjmp threaded interpreter -Og                                         7.2

Tail-call optimized is not reported for -O0 or -Og because it immediately crashes.  
I don't understand that last one, but it was pretty consistent.

   I want to talk about debugging performance. The tail-call optimized version is fast,
   but you can't run it in debugging mode without sacrificing debugging ability.
   The switch wins out in the gap between the performance between debugging and production
   code. If debuggability is a driving concern, the performance hit may be worth it.
