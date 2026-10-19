# HW1 — Custom Linux kernel and system calls

Target: Linux 6.1.0, RISC-V, QEMU. The saved configuration identifies GCC 13.3.0 and GNU binutils 2.42; the package list alone does not pin the entire environment.

## Implementation

- `revstr` (exercise syscall 451) copies a string into kernel memory, reverses it, logs both forms, and copies it back.
- `tempbuf` (exercise syscall 452) manages a kernel linked list of strings with ADD, REMOVE and PRINT operations; user/kernel transfer uses uaccess helpers and list operations use a mutex.
- The patch includes syscall declarations, asm-generic registration, Kbuild entries, implementation directories and their Makefiles. The commented `core-y` additions are not the active build wiring: the submitted implementation uses Kbuild `obj-y`.

These syscall numbers belong to this exercise's Linux 6.1 tree; they are not a portable syscall ABI for arbitrary kernels.

## Included material

- [Kernel patch](patches/0001-add-revstr-tempbuf.patch): diff payload preserved; identifying author metadata replaced.
- [Kernel configuration](config/kernel.config): only identifying local-version suffix changed to `-os-labs`.
- [Package selections](config/packages-list.txt).
- [Historical evidence](evidence/README.md).

## Reproduction prerequisites

Obtain a clean Linux v6.1 tree, the RISC-V GNU toolchain and the course-compatible initramfs. Check the patch with `git apply --check` in that tree before applying it, copy `config/kernel.config` to its `.config`, and build for `ARCH=riscv` with `CROSS_COMPILE=riscv64-linux-gnu-`. These are preparation notes, not a tested build transcript.

The original user-space syscall test sources, kernel tree, built Image and initramfs are not in the supplied submission. The report screenshots do not replace those dependencies. A rebuild using the public configuration will have a different local-version suffix from the identifying original.

## Known limitations

The archived `tempbuf` PRINT path releases the mutex between calculating output size and copying the list. Concurrent changes can invalidate the allocated size or copied length. The ADD path also logs its string after unlocking; concurrent removal can invalidate that pointer. Length arithmetic such as `n + 1` and `size + 1` is not fully guarded. These are static observations, not reproduced failures, and were not repaired during publication.

Next validation: clean-tree patch application/build, small user-space smoke tests in QEMU, then concurrent ADD/REMOVE/PRINT and length-boundary tests. See [course context](https://hackmd.io/@fLANt9b6TbWx5I3lYKkBow/S17gcgmKxl); the live page has since been revised for 2026.
