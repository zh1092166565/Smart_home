#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>


#define MMA_NAME "i2c_mma8451q"
/**********************mma8451 reg********************************/
#define XYZ_STATUS 0x00
#define OUT_X_MSB 0x01
#define OUT_X_LSB 0x02
#define OUT_Y_MSB 0x03
#define OUT_Y_LSB 0x04
#define OUT_Z_MSB 0x05
#define OUT_Z_LSB 0x06

#define WHO_AM_I 0x0d
#define CHIP_ID  0x1a
#define XYZ_DATA_CFG 0x0e
#define CTRL_REG1 0x2a
#define CTRL_REG2 0x2b

/*****************************************************************/
int major;
struct class *cls;
struct device *dev;
struct i2c_client *gclient;


int i2c_write_reg(struct i2c_client *client,char reg,char val)
{
	int ret;
	char wbuf[] = {reg,val};
	struct i2c_msg w_msg[1] = {
		[0] = {
			.addr  = client->addr,
			.flags = 0,
			.len   = 2,
			.buf   = wbuf,
		},			
	};
	ret = i2c_transfer(client->adapter, w_msg, ARRAY_SIZE(w_msg));
	if(ret != ARRAY_SIZE(w_msg)){
		printk("send msg error\n");
		return -EAGAIN;
	}
		
	return 0;
}

int i2c_read_reg(struct i2c_client *client,char reg)
{	
	int ret;
	char val;
	char rbuf[] = {reg};
	struct i2c_msg r_msg[] = {
		[0] = {
			.addr  = client->addr,
			.flags = 0,
			.len   = 1,
			.buf   = rbuf,
		},
		[1] = {
			.addr  = client->addr,
			.flags = I2C_M_RD,
			.len   = 1,
			.buf   = &val,
		},
	};
	ret = i2c_transfer(client->adapter, r_msg, ARRAY_SIZE(r_msg));
	if(ret != ARRAY_SIZE(r_msg))
	{
		printk("receive msg error\n");
		return -EAGAIN;
	}
	return val;
}

int i2c_open (struct inode *inode, struct file *file)
{
	printk("%s,%s,%d\n",__FILE__,__func__,__LINE__);
	return 0;
}
ssize_t i2c_read (struct file *file, char __user *ubuf , size_t size, loff_t *offs)
{
	int ret,i;
	char kbuf[6] = {0};
	printk("%s,%s,%d\n",__FILE__,__func__,__LINE__);
	
	//等待数据转化完成，
	while(!(i2c_read_reg(gclient,XYZ_STATUS) & 0x08));
	//读取数据
	for(i = 0; i < ARRAY_SIZE(kbuf); i++)
	{
		kbuf[i] = i2c_read_reg(gclient, OUT_X_MSB + i);
	}
	//将数据拷贝到用户空间
	if(size > sizeof(kbuf))
	{
		size = sizeof(kbuf);
	}

	ret = copy_to_user(ubuf,kbuf,size);
	if(ret)
	{
		printk("copy_to_user error\n ");
		return -EINVAL;
	}
	
	return size;
}
ssize_t i2c_write (struct file *file, const char __user *ubuf, size_t size, loff_t *offs)
{
	printk("%s,%s,%d\n",__FILE__,__func__,__LINE__);
	return 0;
}
int i2c_close (struct inode *inode, struct file *file)
{
	printk("%s,%s,%d\n",__FILE__,__func__,__LINE__);
	return 0;
}


const struct file_operations i2c_fops = {
	.open = i2c_open,
	.read = i2c_read,
	.write = i2c_write,
	.release = i2c_close,

};




int mma8551q_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	
	int ret;
	printk("%s,%s,%d\n",__FILE__,__func__,__LINE__);
	gclient = client;
	//校验WHO_AM_I
	ret = i2c_read_reg(client,WHO_AM_I);
	printk("WHO_AM_I(0x1a) =%#x\n",ret);

	//2.初始化mma8451芯片
	i2c_write_reg(client,CTRL_REG2,0x40);  //进行软件复位
	i2c_write_reg(client,XYZ_DATA_CFG,0x2); //土8g
	i2c_write_reg(client,CTRL_REG1,0x1);    //上电工作
	i2c_write_reg(client,CTRL_REG2,0x2);    //设置了高精度模式


	//注册字符设别驱动
	major = register_chrdev(0,MMA_NAME,&i2c_fops);
	if(major < 0)
	{
		printk("register_chrdev error\n");
		return major;
	}

	//创建设别节点
	cls = class_create(THIS_MODULE, MMA_NAME);
	if(IS_ERR(cls))
	{
		printk("class_create error\n");
		return PTR_ERR(cls);
	}

	dev = device_create(cls,NULL,MKDEV(major, 0), NULL,MMA_NAME);
	if(IS_ERR(dev))
	{
		printk("device_create error\n");
		return PTR_ERR(dev);
	}
	
	return 0;
}
int mma8451q_close(struct i2c_client *client)
{
	printk("%s,%s,%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

const struct i2c_device_id mma8251q_idtable[] = {
	{"mma8451q",10},
	{/*end*/},
};

struct i2c_driver mma8451q = {
	.probe = mma8551q_probe,
	.remove = mma8451q_close,
	.driver = {
		.name = "i2c_test",
	},
	.id_table = mma8251q_idtable,
};

module_i2c_driver(mma8451q);
MODULE_LICENSE("GPL");
