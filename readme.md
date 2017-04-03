# Binary Toolkit

Binary Toolkit is my latest formal binary analysis framework. For the moment, I am working on ensuring the IR is clean and functional, and building out the basic components for performing analysis.

## ARM

Working on translating ARM. Current instructions as of 03APR2017:

  * ADC
  * ADD

## Should I care?

Not yet. If things work out well, I will begin Semantic Versioning. If you see
an 0.1 release, that's the time to begin caring.

## One IR for everything

BT's IR should be:

  * Usable for multiple tasks, such as jit recompilation across architectures, analysis, translation to SMTLIB2, etc.
  * Easily extensible, so that additional analysis can be easily, "Tacked on," without requiring hacking of the original framework.
  * Implemented in C, and then interacted with through scripting engines. While I have originally used lua for scripting analysis, I am leaning towards duktape and guile for this project.

 Some decisions evident now towards this effect are:

  * A clean object-oriented implementation in C, with basic data structures, based off that which I created during (https://github.com/endeav0r/rdis).
  * Arithmetic operations operate over operands of the same bit-width. Truncate, zero-extend, and sign-extend are used extensively.
  * All reads and writes are 8-bytes, and read/writes of a larger site are expanded during translation. **(This may change because it's super annoying)**
  * No explicit definition of a target architecture is required for JIT. JIT will just run.

## What works

  * JIT from HSVM (https://github.com/endeav0r/hsvm) to amd64.
  * Several underlying requirements for further development.

## What to expect

This is a freetime hacking project of mine. I'm not sure where it will lead, but if results look promising I will pursue documentation.

If I publish translators from real architectures, those translators will be most likely implemented on top of capstone.

## Other documents

http://tfpwn.com/binary_translation.pdf
