# N19 Reference Compiler

Made to compile fast and produce not great, but acceptable output. Currently only 386 output supported (protected and partially real mode).

# Installation

    # Use gas syntax
    make GAS=1
    # or nasm
    make
    
    sudo make install

# Command-line usage

    ntc arg1=val1 arg2=val2 ... argn=valn

# Valid arguments

This will be ported into a man page later:

    x86_target: Target processor feature set (0 for 8086, 3 for 80386, m for generic x86_64)
    x86_mode: Target operating mode (16 for 16-bit real mode, 32 for 32-bit protected mode or long mode, 64 for 64-bit long mode)
    in: Input Nectar source file

Unknown arguments are ignored.

# License

nctref is licensed under GPL3.