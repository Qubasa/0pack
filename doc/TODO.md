## Currently Working
* Data, Rodata and Text encryption
each gets stored in a separate custom .debug section

* Symbol section with random virtual addresses

* Zero as entrypoint confuses radare and sets the entrypoint to .text

* Sections without flags still get loaded. Why?

## Wanted 
* Encrypted sections should not be appended to the end
* Entrypoint should first be x bytes of zeros which get interpreted as add instructions
* Assembler code should not use stack to confuse dissasemlbers. And use rbp and rsp as general purpose registers. 
* Dynamic relocations should change decryptor code and should change bytes in the encrypted sections.
* Dynamic relocation should change itself (if possible)
* cpuid check jumps to dummy code
* Decryption routine should be a mix of propper encryption and a sliding window
* Add junk code generator
* Change section size to hide encrypted data
* Dummy elf does not need to work
* Make decryptor stub polymorphic
* Add a virtual machine (also polymorphic)
* Add unroling decryption
