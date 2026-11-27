# Win-To-Linux OS build

This is the independent distribution build foundation. Ubuntu is a build host only; the released ISO must contain its own kernel, bootloader, root filesystem, desktop/session, package tools and installer.

Run a read-only host inventory with `./os/scripts/inventory-host.sh`. Output is stored under `os/manifests/`. This does not install, remove, or change host packages.

See [BUILD-DESIGN.md](docs/BUILD-DESIGN.md).
