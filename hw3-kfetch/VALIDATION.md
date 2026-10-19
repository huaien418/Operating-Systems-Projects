# kfetch validation plan

Status on 2026-10-19: archived source; not newly compiled, loaded or executed.

## Static limitations preserved in this version

1. `kfetch_read` copies its entire generated output without respecting the caller's `length`; it also returns EOF for every positive offset rather than supporting partial reads.
2. `separator_line[64]` is filled and terminated using the full hostname length. A 64-character hostname can place the terminator outside that array.
3. `for_each_process` is traversed without RCU read-side protection. The module's mutex does not protect the kernel's global process list.
4. Error paths and output bounds require further testing. The presence of a mutex alone does not establish correctness for all concurrent callers.

These observations are recorded for review. They are not fixed or demonstrated exploits in this archive.

## Next demonstration

- Address the read/offset, bounds and process-list synchronization issues in a separately documented revision.
- Build against the exact Linux 6.8 RISC-V devkit and record compiler, kernel and source revision.
- In a disposable QEMU guest, record load/unload exit status and kernel messages, then test the staff user-space client if available.
- Exercise all mask bits, empty mask, default-state persistence, zero/short reads, invalid write sizes and concurrent callers.
- Save actual stdout and kernel logs with dates. New evidence should be labeled as a new validation run rather than a historical course screenshot.

References: [kernel RCU process-list example](https://docs.kernel.org/6.8/RCU/listRCU.html) and [external module builds](https://docs.kernel.org/6.8/kbuild/modules.html).
