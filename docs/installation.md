# Build, install, run, and remove

Win-To-Linux currently provides a Qt/QML desktop-shell preview. It runs as your user inside an existing Wayland session; it does not yet replace GNOME/KDE, provide its own login session, or include a standalone compositor.

## Build and test

On Ubuntu 24.04, install `build-essential`, `cmake`, `ninja-build`, `qt6-base-dev`, `qt6-declarative-dev`, `qt6-declarative-dev-tools`, `qml6-module-qtqml`, `qml6-module-qtqml-models`, `qml6-module-qtqml-workerscript`, `qml6-module-qtquick`, `qml6-module-qtquick-controls`, `qml6-module-qtquick-templates`, `qml6-module-qtquick-layouts`, `qml6-module-qtquick-window`, and `qml6-module-qtquick-dialogs`. Then run:

```sh
sh scripts/build.sh
```

## Install for the current user

Run `sh scripts/install-user.sh` without `sudo`. The script builds/tests, installs to `~/.local`, and installs a user service at `~/.config/systemd/user/Win-To-Linux.service` (or `$XDG_CONFIG_HOME/systemd/user` if configured).

Before starting the nested preview, run the read-only readiness check:

```sh
~/.local/bin/win-to-linux --preview-check
```

It lists every missing prerequisite without starting a compositor. Exit code `0` means the nested preview prerequisites are present; exit code `2` means it is not safe/ready to launch. It never installs packages or changes the login manager.

Start/restart from a terminal inside an existing Wayland desktop session:

```sh
systemctl --user daemon-reload
systemctl --user start Win-To-Linux.service
# later:
systemctl --user restart Win-To-Linux.service
systemctl --user status Win-To-Linux.service
journalctl --user -u Win-To-Linux.service -e
```

The service is deliberately user-scoped. A graphical app should not be launched as root, and a system-level `systemctl restart Win-To-Linux` unit cannot safely infer which logged-in user's Wayland session to attach to. The supported command is therefore `systemctl --user restart Win-To-Linux.service`. Start it only from an active Wayland desktop with Cage installed; the launcher refuses unsafe direct-display/TTY starts. The service is not enabled automatically.

## Uninstall

Run `sh scripts/uninstall-user.sh`. It stops/removes the user service and installed launchers. Source, build output, and user settings are left untouched.

## Safety and current scope

Do not run these scripts with `sudo`. The application runs in the current user's desktop session. This preview installer does not install a compositor or modify GDM, SDDM, login/session managers, or system packages automatically. Windows `.exe` support would require a separately configured compatibility layer such as Wine and is not implied by this preview.
