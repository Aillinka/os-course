# Practical Work 1 — Linux Administration

**Course:** Operating Systems
**Level:** C
**Environment:** Ubuntu Linux

## Objective

To configure secure remote access, investigate Linux
initialization systems, create a custom systemd service,
and analyze operating system boot performance.

## Task 1 — SSH Configuration

OpenSSH Server was installed and enabled.

SSH key-based authentication was configured to connect
from macOS to Ubuntu.

## Task 2 — Initialization Systems

systemd and traditional SysV init were compared.

The comparison is available in init_comparison.md.

## Task 3 — Custom systemd Service

The Python script clock.py records the current time
every 10 seconds.

The machine-clock.service unit manages the script
and supports automatic startup.

### Installation

Run the following command from the pr01 directory:

    ./run.sh

### Verification

    systemctl is-active machine-clock.service
    systemctl is-enabled machine-clock.service
    journalctl -u machine-clock.service -n 10

## Task 4 — Boot Analysis

The following commands were used:

    systemd-analyze
    systemd-analyze blame

Results are stored in boot_time.txt and boot_services.txt.

Possible improvements include reviewing unnecessary
services and disabling them only after checking
their dependencies and purpose.

## Conclusion

This practical work demonstrated Linux service
administration and remote access configuration.

SSH key authentication enabled remote access from macOS.

A custom systemd service was implemented and
configured for automatic startup.

Boot analysis tools were used to investigate
system startup performance.

## Actual Experimental Results

The Ubuntu system boot time was 16.461 seconds:
- Kernel: 0.153 seconds
- Initrd: 1.479 seconds
- Userspace: 14.828 seconds

The systemd service was successfully tested after reboot.

Service status: active
Autostart status: enabled

The system clock was synchronized using NTP.
Time zone: Asia/Bishkek.

## Boot Optimization Recommendations

The boot analysis identified snapd.service,
plymouth-quit-wait.service, and fwupd.service
as services with notable startup durations.

The snapd service should be investigated to
determine whether Snap applications are required.

The Plymouth startup wait may also be reviewed
in a minimal server environment.

No services were disabled during the experiment
because their dependencies must be checked first.

## Final Result

The SSH connection, Python clock logger,
systemd service, automatic startup, and boot
performance analysis were successfully tested.
