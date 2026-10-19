# Validation status

Prepared on 2026-10-19. This publication is an archival presentation, not a new runtime validation.

| Area | What is established | What remains unverified |
| --- | --- | --- |
| HW1 | Patch contains syscall registration, prototypes, Kbuild entries and two implementations; historical images show example outputs | Applying/building on a clean kernel, current QEMU boot, error paths and concurrency |
| HW2 | Source uses pthread policy attributes, CPU0 affinity requests, barrier synchronization and thread CPU time; historical report contains output and test observations | Current build and execution, affinity success, privileges, official reference comparisons |
| HW3 | Source contains device lifecycle and open/release/read/write callbacks with information mask | Compilation against the exact devkit, load/unload, output correctness, concurrency and boundary tests |

Publication checks cover file identity, documented privacy substitutions, patch structure, build-target/source naming, relative documentation links and selected-image inspection. These checks do not establish runtime correctness.

Known implementation limitations are preserved in the [HW1](hw1-system-calls/README.md), [HW2](hw2-scheduling/README.md), and [HW3](hw3-kfetch/VALIDATION.md) notes. Kernel experiments should be reproduced in a disposable, version-matched QEMU guest. No system scheduling settings were changed for this publication.
