#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x7a98f4b4, "copy_from_user_nofault" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x53569707, "this_cpu_off" },
	{ 0x69acdf38, "memcpy" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0xe2964344, "__wake_up" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x92997ed8, "_printk" },
	{ 0x1000e51, "schedule" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x1449bb6c, "const_pcpu_hot" },
	{ 0x167c5967, "print_hex_dump" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0x35273366, "unregister_kretprobe" },
	{ 0x75ca79b5, "__fortify_panic" },
	{ 0x50e3d66d, "misc_register" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0x3f66a26e, "register_kprobe" },
	{ 0xceb38aab, "register_kretprobe" },
	{ 0xbb10e61d, "unregister_kprobe" },
	{ 0xe2c17b5d, "__SCT__might_resched" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x59eebd26, "misc_deregister" },
	{ 0xe2e2ca37, "module_layout" },
};

MODULE_INFO(depends, "");

