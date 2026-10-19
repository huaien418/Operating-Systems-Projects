# HW2 — Scheduling policy demonstration

A C/pthread experiment comparing NORMAL (`SCHED_OTHER`) and FIFO (`SCHED_FIFO`) policies on one intended CPU.

## Implementation

The main thread parses policy/priority lists, requests CPU0 affinity, sets explicit pthread scheduling attributes, creates workers and releases a shared start barrier. Each worker prints its ID three times, with CPU-time busy work between messages. `CLOCK_THREAD_CPUTIME_ID` measures work performed by that thread rather than elapsed time while preempted.

## Building and running

Use Linux with pthread support. For the course RISC-V target, run in the matching build environment:

```sh
make CC=riscv64-linux-gnu-gcc
```

For a Linux host build, `make CC=gcc` uses the same source but does not validate RISC-V execution. Dynamic binaries require the corresponding libraries in the guest rootfs. Once the binary and runtime are available in a disposable guest, the historical examples use:

```sh
./sched_demo -n 3 -t 1.0 -s NORMAL,FIFO,FIFO -p -1,10,30
./sched_demo -n 4 -t 0.5 -s NORMAL,FIFO,NORMAL,FIFO -p -1,10,-1,30
```

FIFO policy creation requires suitable privileges/limits. CPU0 must be allowed by the guest's CPU affinity constraints. The above commands are examples, not newly executed tests. Scheduling order can depend on runtime settings and workload.

The actual Makefile compiles one C source directly; it does not generate the `.o`/`.d` dependency scheme discussed in the original report. Only the identifying target/source names were changed for publication.

## Evidence and gaps

See [historical output and test observations](evidence/README.md). The original `sched_test.sh` and staff reference executable are absent from the supplied ZIP, so those comparisons were not rerun. No privileged scheduling experiments or RT-throttling changes were performed for this publication.

Known limitations: the archived source does not check the return value of `pthread_setaffinity_np`, so a failed CPU0 pin could invalidate the single-CPU premise. Several allocation, attribute and barrier calls lack checks. CLI conversion and thread-count bounds also need stronger validation. These remain follow-up work, not claimed fixes.

Reference: [TA assignment specification](https://hackmd.io/@seco1024/Bk48CgNClg).
