certaines fonctions du kernel ne sont pas appellable par des modules:

root@debian:/home/debian/IPC/Mini_Kit# grep within_kprobe_blacklist /usr/src/linux-headers-*/Module.symvers
root@debian:/home/debian/IPC/Mini_Kit# cat /proc/kallsyms | grep within_kprobe_blacklist
ffffffff86fffeb0 W __pfx_arch_within_kprobe_blacklist
ffffffff86fffec0 W arch_within_kprobe_blacklist
ffffffff86fffef0 T __pfx_within_kprobe_blacklist
ffffffff86ffff00 T within_kprobe_blacklist

en fait sur certains point les modules kernels ont moins de droits que le root ...


root@debian:/home/debian/IPC/Mini_Kit# cat /proc/kallsyms | grep page_offset_base
ffffffff997e55a8 D page_offset_base
ffffffff9986cac8 r __ksymtab_page_offset_base
