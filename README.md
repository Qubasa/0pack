ELF Obfuscator
================================================

# Description
An ELF x64 binary payload injector written in c++ using the LIEF library.
Injects shellcode written in fasm as relocations into the header.
Execution begins at entrypoint 0 aka the header, this confuses or downright brakes debuggers.
The whole first segment is writeable, this can be mitigated at runtime through an injected payload which sets the binaries segment to just rx.

# Compiler flags
The targeted binary must have following flags:
gcc -m64 -fPIE -pie

Statically linking is not possible as -pie and -static are incompatible flags.
Or in other terms:
```
-static means a statically linked executable with no dynamic
> relocations and only PT_LOAD segments.  -pie means a shared library with
> dynamic relocations and PT_INTERP and PT_DYNAMIC segments.
```

## Debugger behaviour
Debuggers don't generally like 0 as the entrypoint and oftentimes it is impossible to set breakpoints at the header area.
Another often occured issue is that the entry0 label gets set incorrectly to the main label which is controllable through debugging symbols.
Which means the attacker can purposely mislead the reverse engineer into reverse engineering fake code.
Executing `db entry0` in radare2 has this behaviour.


## Affected debuggers
* radare2
* Hopper
* gdb
* IDA Pro --> Not tested

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
