# Independent distribution build design

## Core rule
Ubuntu is the build host only. The released ISO must contain its own bootloader, kernel, initramfs, root filesystem, desktop/session, package tools and installer. Nothing in the installed system may resolve executables, libraries or configuration from the machine that built the ISO.

Host inventory is a source catalogue, not a root filesystem. We record available tools, installed package versions, kernel configuration and executable paths, then assemble a clean target root. Copying `/usr` wholesale is forbidden because it imports host-specific state, absolute symlinks, package database assumptions and ABI coupling.

## Layers
1. Build-host adapter: inventory tools and packages without modifying the running desktop, display manager, kernel or host package set.
2. Kernel pipeline: fetch a pinned upstream Linux source, apply tracked config/patches, build kernel and modules. Never install it on the build host.
3. Target rootfs: assemble under `out/rootfs` from explicit package manifests. Reuse host binaries only after dependency closure, ABI, licensing and paths are validated. Prefer extracting packages into the target root.
4. Package compatibility: DEB backend (dpkg/apt) and RPM backend (rpm/dnf) behind one UI/API. They must have separate package databases and file ownership. Cross-family packages are not automatically portable.
5. Desktop and app runtimes: native Linux apps; Windows via Wine/Proton; Android via a Waydroid-style container where kernel/binder support exists.
6. Installer/ISO: bootable UEFI/BIOS image, offline installation, checksums and no build-host mount/bind dependency.

## Package policy
A `.deb` or `.rpm` is not a portable bundle. Dependencies may require different libc, services, filesystem layouts or versions. Inspect metadata and dependencies before installation. Refuse unsupported/conflicting packages rather than force-installing. Isolated compatibility roots may host foreign-family apps without contaminating the base system.

## Current milestone
- Capture Ubuntu host package/tool inventory read-only.
- Version-control target package seeds and configuration.
- Validate manifests and scripts in CI.
- Build isolated rootfs and ISO only after ABI, package ownership and boot strategy are pinned.

## Safety gates
- Never replace host kernel or bootloader during development.
- Never run installer/display-manager changes against the development VM.
- All build output stays below `out/`.
- Pin and checksum all downloaded sources.
- Test ISO in a disposable VM, not on the development desktop.
