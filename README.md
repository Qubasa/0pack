ELF Obfuscator
================================================

# Description
An ELF x64 binary obfuscator written in c++ using the LIEF library.
Adds a new segment without a corrisponding section. Sets  the entrypoint to the new segment. 
Encrypts the original .text section. The new segment decrypts in memory the .text section.

# Compiler flags
The binary to be obfuscated must have following flags:
gcc -m64 -fPIE -pie

Statically linking is not possible as -pie and -static are incompatible flags. 
Or in other terms:
```
-static means a statically linked executable with no dynamic
> relocations and only PT_LOAD segments.  -pie means a shared library with
> dynamic relocations and PT_INTERP and PT_DYNAMIC segments.  <Paste>
```

# Technical notes
The obfuscator assumes that the .text section and the .rodata section are all in the first segment. If this is not the case the binary will segfault.

## Dependencies
* cmake
  version 3.12.2 or higher
* build-essential
* gcc
* fasm


### Use build script
`
    $ ./build.sh
`

### Build it manually
```
  $ mkdir build
  $ cd build
  $ cmake ..
  $ make
  $ ./../main.elf
```
