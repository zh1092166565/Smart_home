#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>


struct resource res[] = {
	[0] ={
			.start = 0xc001a000,
			.end = 0xc001a000 + 3,
			.flags = IORESOURCE_IO,
		},
	[1] = {
			.start = 0xc001e000,
			.end = 0xc001e000 + 3,
			.flags = IORESOURCE_IO,
		},
	[2] = {
			.start = 0xc001b000,
			.end = 0xc001b000 + 3,
			.flags = IORESOURCE_IO,
		},
	[3] = {
			.start = 0xc001c000,
			.end = 0xc001c000 + 3,
			.flags = IORESOURCE_IO,
		}

};


void pdev_release(struct device *dev)
{
	printk("%s,%s,%d\n",__FILE__,__func__,__LINE__);
}



struct platform_device pdev ={
	.name = "a53_led",
	.id = -1,
	.dev = {
			.release = pdev_release,
		},
	.resource = res,
	.num_resources = ARRAY_SIZE(res),
};




static int __init pla_led_pdev_init(void)

{
	return platform_device_register(&pdev);
}

static void __exit pla_led_pdev_exit(void)

{
	platform_device_unregister(&pdev);
}

module_init(pla_led_pdev_init);
module_exit(pla_led_pdev_exit);

MODULE_LICENSE("GPL");





