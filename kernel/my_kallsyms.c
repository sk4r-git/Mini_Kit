#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>
#include <linux/fdtable.h>
#include <linux/delay.h>
#include <linux/kprobes.h>
#include <linux/uio.h>
#include <linux/kallsyms.h>
#include <asm/processor.h>
#include <asm/paravirt.h>
#include <linux/uaccess.h>
#include <linux/mm.h>



static int safe_read(void *dst, void *src, size_t len)
{
    return copy_from_kernel_nofault(dst, src, len);
}

static int is_mapped(unsigned long addr)
{
    unsigned char x;
    return !copy_from_kernel_nofault(&x, (void*)addr, 1);
}

static int find_elf_header(unsigned long addr)
{
    unsigned char buf[4];

    if (safe_read(buf, (void *)addr, sizeof(buf)) == 0) {
        if (buf[0] == 0x7f && buf[1] == 0x45 &&
            buf[2] == 0x4C && buf[3] == 0x46) {
            printk("hit elf_header at %lx\n", addr);
            return 1;
        }
    }

    return 0;
}


static int looks_like_text(unsigned long addr)
{
    unsigned char buf[64];
    if (safe_read(buf, (void*)addr, sizeof(buf)))
        return 0;

    int score = 0;
    for (int i = 0; i < 64; i++) {
        if (buf[i] != 0x00 && buf[i] != 0xff)
            score++;
    }

    return score > 48;
}

#define SCAN_SIZE 0x200000000UL 
extern unsigned long page_offset_base;

unsigned long find_text_base(void)
{
    unsigned long elf_header = 0;
    printk(KERN_INFO "page offset base at %lx\n", page_offset_base);
    for (unsigned long a = page_offset_base; a < page_offset_base + SCAN_SIZE; a += 0x1000) {
        if (!is_mapped(a))
            continue;
        if (find_elf_header(a)){
            elf_header = a;
            break;
        }
    }
    if (elf_header == 0){
        return 0;
    }
    for (unsigned long a = elf_header; a < elf_header + SCAN_SIZE; a += 0x1000) {
        if (!is_mapped(a))
            continue;
        if (looks_like_text(a))
            return a;
    }
    return 0;
}

int looks_like_rodata(unsigned long addr)
{
    unsigned char buf[50];
    int ascii = 0;

    if (safe_read(buf, (void *)addr, sizeof(buf)) == 0){
        for (int i = 0; i < sizeof(buf); i++) {
            if (buf[i] >= 0x20 && buf[i] <= 0x7e)
                ascii++;
        }
    }

    return ascii > 40;
}

unsigned long find_rodata(unsigned long text)
{
    unsigned long a;

    for (a = text; a < text + SCAN_SIZE; a += 0x1000) {
        if (looks_like_rodata(a))
            return a;
    }
    return 0;
}

unsigned long get_addresse_of_symbol_from_string(const char * name){
    // unsigned long maybe_text_base = find_text_base();
    // printk(KERN_INFO "maybe we find kernel .text at %lx\n", maybe_text_base);
    // unsigned long maybe_rodata = find_rodata(maybe_text_base);
    // printk(KERN_INFO "maybe we find kernel .rodata at %lx\n", maybe_rodata);
    // printk(KERN_INFO "maybe we find first rodatastr at %s\n", maybe_rodata);
    // get the adresse of the check function
    static struct kprobe kp_t = {
        .symbol_name = "within_kprobe_blacklist",
    };
    register_kprobe(&kp_t);
    unsigned long * within_kprobe_bl = (unsigned long *) kp_t.addr;
    within_kprobe_bl -= 4;
    unregister_kprobe(&kp_t);
    printk(KERN_INFO "vfs read kprobe is at %lx\n", within_kprobe_bl);
    // printk(KERN_INFO "vfs read kprobe is at %lx, %lx\n", *within_kprobe_bl, *(within_kprobe_bl + 8));
    
    //
    return 0;
}