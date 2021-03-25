#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/ioport.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#define GPIONO(m,n) (m*32+n)
#define RED GPIONO(0,28)
#define GREEN GPIONO(4,13)
#define BLUE GPIONO(1,12)

unsigned int led[] = {RED,GREEN, BLUE};


//设计全局的设别队形
struct my_led_dev
{
	int dev_major;
	struct class * cls;
	struct device *dev;
	struct resource *res[4];

	void *reg_bas[4];

	
};

struct my_led_dev *my_led;


int led_dev_open (struct inode *inode, struct file *file)
{
	printk("%s,%s,%d\n",__FILE__,__func__,__LINE__);
	return 0;

}

ssize_t led_dev_read (struct file *file, char __user *buf, size_t count, loff_t *fpos)
{
	printk("%s,%s,%d\n",__FILE__,__func__,__LINE__);
	return 0;
}
ssize_t led_dev_write (struct file *file, const char __user *buf, size_t count, loff_t *fops)
{

	int val = 0;
	int ret;
	printk("%s,%s,%d\n",__FILE__,__func__,__LINE__);
	ret = copy_from_user(&val, buf, count);
	printk("pdrv = %d\n",val);
	if(ret > 0)
	{
		printk("copy_from_user error \n");
		return -EFAULT;
	}
	if (val > 0)
	{	int flg;
		writel(readl(my_led->reg_bas[0]) | (1<<28), my_led->reg_bas[0]);
		writel(readl(my_led->reg_bas[3]) | (1<<14), my_led->reg_bas[3]);
		flg = readl(my_led->reg_bas[0]);
		printk("flg = %d\n",flg);
		printk("on led\n");
		//writel(readl(my_led->reg_bas[2] + 0) | (1<<12), my_led->reg_bas[2] + 0);
	}
	else 
	{	
		int flg;
		writel(readl(my_led->reg_bas[0]) & ~(1<<28), my_led->reg_bas[0]);
		flg = readl(my_led->reg_bas[0]);
		printk("flg = %d\n",flg);
		printk("off led\n");
		//writel(readl(my_led->reg_bas[2] + 0) & ~(1<<12), my_led->reg_bas[2] + 0);
	}

	return count;
}
int lec_dev_close (struct inode *inode, struct file *file)
{
	printk("%s,%s,%d\n",__FILE__,__func__,__LINE__);
	return 0;
}


const struct file_operations  fops={
	.open = led_dev_open,
	.read = led_dev_read,
	.write = led_dev_write,
	.release = lec_dev_close,

};




int led_prv_probe(struct platform_device *pdev)


{
	//int ret;
	//int i;
	printk("%s,%s,%d\n",__FILE__,__func__,__LINE__);
	my_led = kzalloc(sizeof(struct my_led_dev), GFP_KERNEL);
	if(my_led == NULL)
	{
		printk("kzalloc my_led error \n");	
		return -ENOMEM;
	}
	/*
		a，注册设备号，并且注册fops--为用户提供一个设备标示，同时提供文件操作io接口
		b， 创建设备节点
		c， 初始化硬件
				ioremap(地址);  //地址从pdev需要获取
				readl/writle();
		d，实现各种io接口： xxx_open, xxx_read, ..
	*/

	my_led->dev_major = register_chrdev(0, "led_dev", &fops);
	if(my_led->dev_major < 0)
	{
		printk("register_chrdev error \n");
		return -EINVAL;
	}

	my_led->cls = class_create(THIS_MODULE, "led_a53_cls");
	my_led->dev = device_create(my_led->cls,NULL, MKDEV(my_led->dev_major, 0),NULL,"led0");

	my_led->res[0] = platform_get_resource(pdev,IORESOURCE_IO, 0);
	my_led->reg_bas[0] = ioremap(my_led->res[0]->start, resource_size(my_led->res[0]));

	my_led->res[1] = platform_get_resource(pdev,IORESOURCE_IO, 1);
	my_led->reg_bas[1] = ioremap(my_led->res[1]->start, resource_size(my_led->res[1]));

	my_led->res[2]= platform_get_resource(pdev,IORESOURCE_IO, 2);
	my_led->reg_bas[2] = ioremap(my_led->res[2]->start, resource_size(my_led->res[2]));

	my_led->res[3]= platform_get_resource(pdev,IORESOURCE_IO, 3);
	my_led->reg_bas[3] = ioremap(my_led->res[2]->start, resource_size(my_led->res[3]));


	writel(readl(my_led->reg_bas[0] + 9) & ~(3<<24) , my_led->reg_bas[0] + 9);	
	writel(readl(my_led->reg_bas[0] + 1) | (1<<28) , my_led->reg_bas[0] + 1);	
	writel(readl(my_led->reg_bas[0] + 0) & ~(1<<28), my_led->reg_bas[0] + 0);

/*
	writel(readl(my_led->reg_bas[1] + 8) & ~(3<<26) , my_led->reg_bas[1] + 8);	
	writel(readl(my_led->reg_bas[1] + 1) | (1<<13) , my_led->reg_bas[1] + 1);	
	writel(readl(my_led->reg_bas[1] + 0) & ~(1<<13), my_led->reg_bas[1] + 0);
*/




	writel(readl(my_led->reg_bas[2] + 8) & ~(3<<24) , my_led->reg_bas[2] + 8);
	writel(readl(my_led->reg_bas[2] + 8) | (1<<25) , my_led->reg_bas[2] + 8);
	writel(readl(my_led->reg_bas[2] + 1) | (1<<12) , my_led->reg_bas[2] + 1);	
	writel(readl(my_led->reg_bas[2] + 0) & ~(1<<12), my_led->reg_bas[2] + 0);

	writel(readl(my_led->reg_bas[3] + 8) & ~(3<<28) , my_led->reg_bas[3] + 8);
	writel(readl(my_led->reg_bas[3] + 8) | (1<<28) , my_led->reg_bas[3] + 8);
	writel(readl(my_led->reg_bas[3] + 1) | (1<<14) , my_led->reg_bas[3] + 1);	
	writel(readl(my_led->reg_bas[3] + 0) & ~(1<<14), my_led->reg_bas[3] + 0);
/*

	//申请gpio
	for(i = 0; i < ARRAY_SIZE(led) ; i++)
	{
		ret = gpio_request(led[i],NULL);
		if(ret)
		{
			printk("gpio request error \n");
			return ret;
		}
		gpio_direction_output(led[i],0);
	}
*/
	return 0;
}
int led_prv_remove(struct platform_device *pdev)
{
	//int i;
	printk("%s,%s,%d\n",__FILE__,__func__,__LINE__);
	iounmap(my_led->reg_bas);
	device_destroy(my_led->cls, MKDEV(my_led->dev_major, 0));
	class_destroy(my_led->cls);
	unregister_chrdev(my_led->dev_major, "led_dev");
	kfree(my_led);
/*
	for(i = 0 ; i < ARRAY_SIZE(led) ; i++)
	{
		gpio_free(led[i]);
	}
*/
	return 0;
	
}

const struct platform_device_id led_id_table[] = {
	{"a53_led",0x4444},
};

struct platform_driver led_drv ={
	.probe = led_prv_probe,
	.remove = led_prv_remove,
	.driver = {
			.name = "samsung_led_dev",
		},
	.id_table = led_id_table,
};


static int __init pla_led_pdrv_init(void)
{
	//注册一个pdrv
	return platform_driver_register(&led_drv);
}
static void __exit pla_led_pdrv_exit(void)
{
	platform_driver_unregister(&led_drv);
}

module_init(pla_led_pdrv_init);
module_exit(pla_led_pdrv_exit);
MODULE_LICENSE("GPL");


