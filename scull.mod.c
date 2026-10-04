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
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x52c5c991, "__kmalloc_noprof" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x4829a47e, "memcpy" },
	{ 0x37a0cba, "kfree" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0xe2964344, "__wake_up" },
	{ 0x92997ed8, "_printk" },
	{ 0x1000e51, "schedule" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x9c1e5bf5, "queued_spin_lock_slowpath" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0x800473f, "__cond_resched" },
	{ 0xd83dafc7, "cdev_add" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x75ca79b5, "__fortify_panic" },
	{ 0xdcb764ad, "memset" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x56470118, "__warn_printk" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0xa65c6def, "alt_cb_patch_nops" },
	{ 0x3fd78f3b, "register_chrdev_region" },
	{ 0xa3439e90, "cdev_init" },
	{ 0xb4ba5c8b, "cdev_del" },
	{ 0x8f213c29, "module_layout" },
};

MODULE_INFO(depends, "");

