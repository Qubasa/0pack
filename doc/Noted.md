## Radare bug
If the entrypoint is 0 the execution begins at page0 but the entry0 flag get's set to the beginning of the .text section.

## KVM Bug?
pop rdi creates a BUS error (7) instead of a Sigsev error (11) on KVM

Could just be because of different Kernel versions though...

