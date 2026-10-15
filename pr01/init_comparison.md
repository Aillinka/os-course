# Comparison of Initialization Systems

| Feature | systemd | SysV init |
|---|---|---|
| Service definition | Unit files | Shell scripts |
| Start service | systemctl start | service start |
| Check status | systemctl status | service status |
| Enable startup | systemctl enable | update-rc.d |
| Logs | journalctl | Syslog and service log files |
| Startup | Dependency-based, supports parallelism | Traditional runlevel startup |

## Conclusion

systemd provides centralized service management,
dependency handling, automatic restarting,
and integrated logging.

Traditional SysV init uses shell scripts and runlevels.
It offers a simpler service structure but fewer
built-in management features.

The experiment was performed on systemd.
SysV init was compared theoretically.
