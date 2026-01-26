savedcmd_/home/debian/IPC/Mini_Kit/kernel/intercept.o := x86_64-linux-gnu-ld -m elf_x86_64 -z noexecstack --no-warn-rwx-segments   -r -o /home/debian/IPC/Mini_Kit/kernel/intercept.o @/home/debian/IPC/Mini_Kit/kernel/intercept.mod  ; ./tools/objtool/objtool --hacks=jump_label --hacks=noinstr --hacks=skylake --ibt --orc --retpoline --rethunk --sls --static-call --uaccess --prefix=16  --link  --module /home/debian/IPC/Mini_Kit/kernel/intercept.o

/home/debian/IPC/Mini_Kit/kernel/intercept.o: $(wildcard ./tools/objtool/objtool)
