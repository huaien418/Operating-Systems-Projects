# Source and evidence notes

## Publication changes

- HW1: author/sign-off metadata in the patch uses the public GitHub identity and GitHub noreply address. The actual diff payload is unchanged. The kernel configuration uses `-os-labs` in place of its identifying local-version suffix. The package list is otherwise unchanged.
- HW2: the source and executable are named `sched_demo`; the Makefile and source-header comment are updated to match. Program logic is unchanged.
- HW3: the source and module are named `kfetch_mod`; the Makefile, log prefix and module author metadata are updated to match. `MODULE_LICENSE("GPL")` is retained. Program logic is unchanged.
- Four report images are included byte-for-byte under neutral filenames, after visual inspection. Images with student IDs, full reports and original submission archives are excluded.

The private preparation manifest records source mappings and SHA-256 hashes. It is kept outside this public repository because its source filenames contain student identifiers.

## Upstream notices

HW1 is a patch against the upstream Linux kernel and must be considered with the upstream licensing notices. HW3 retains its existing GPL module declaration. No blanket MIT license or new license for course materials has been added. See the upstream [Linux COPYING](https://github.com/torvalds/linux/blob/v6.1/COPYING) and [license rules](https://docs.kernel.org/process/license-rules.html) for the kernel's own notices.

The archived package selection list records package names and installation selections, not exact package versions or a reproducible container image.
