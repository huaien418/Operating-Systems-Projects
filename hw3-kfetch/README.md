# HW3 — kfetch kernel module

An archived character device module intended for the course's Linux 6.8.0 RISC-V QEMU environment.

## Source implementation

The source registers a character device and creates `/dev/kfetch`, implements open/release/read/write callbacks, and formats an ASCII logo alongside system information. A bitmask selects kernel release, CPU count/model, memory, uptime and process count; a mutex protects the shared mask and output buffer. The source follows the assignment's interface and logo.

## Build prerequisites

The supplied Makefile expects the course's matching kernel development kit, `ARCH=riscv` and a `riscv64-linux-gnu-` toolchain. From this project directory, with the devkit obtained separately:

```sh
make KDIR=/absolute/path/to/linux-v6.8-devkit
```

The public module target is `kfetch_mod.ko`. The development kit must match the guest kernel and host-tool architecture; it is not included here. Neither the course user-space `kfetch` program nor a kernel Image/initramfs is included. Build instructions are unexecuted preparation notes.

## Evidence status

The supplied HW3 archive contains source and Makefile only. Historical runtime screenshots were not retained in these files, and no replacement output has been fabricated. This project is presented as source implementation pending fresh verification. Read [known issues and validation plan](VALIDATION.md) before attempting a demonstration.

Reference: [TA assignment specification](https://hackmd.io/@seco1024/BJYIDp7lZx), including the Linux 6.8 environment and device protocol.
