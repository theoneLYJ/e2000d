#include <linux/init.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

MODULE_LICENSE ("GPL");
unsigned long page_addr = 0;
void *kmalloc_addr = 0;
int err = 0;

int __init mem_init (void)
{
	/*申请8个物理内存页*/
	page_addr = __get_free_pages (GFP_KERNEL, 3);
	if (!page_addr) {
		panic ("cannot alloc so much pages!");
		err =  -ENOMEM;
		goto failure_get_free_pages;
	}
	printk (KERN_ERR "page_addr = %p phy addr = %p\n", page_addr, virt_to_phys (page_addr));

	kmalloc_addr = kmalloc (100, GFP_KERNEL);
	if (!kmalloc_addr) {
		err = -ENOMEM;
		goto failure_kmalloc;
	}
	printk (KERN_ERR "kmalloc_addr = %p phy addr = %p\n", kmalloc_addr, virt_to_phys (kmalloc_addr));

failure_kmalloc:
	kfree (kmalloc_addr);
failure_get_free_pages:
	return err;
	return 0;
}
void __exit mem_exit (void)
{
	kfree (kmalloc_addr);
	free_pages (page_addr, 3);
	return ;
}
module_init (mem_init);
module_exit (mem_exit);
