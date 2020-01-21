# example-bin-loader
A simple example binary loader copying executable binary from rodata section to memory and execute from memory

The `myprogram.bin` is provided to allow you to upload your own binary executable that you wishes to run after the bin-loader copy and jump to it. This executable binary of course need to be striped off any ELF meta-data, by using simalar utility such as:
```
    riscv64-unknown-elf-objcopy -O binary program.elf program.bin
```
