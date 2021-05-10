/*
 *  i2c_bus
 *  
 *  (C) 2021.04.29 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/io.h>

#define I2C_DEMO_C		0x0
#define I2C_DEMO_S		0x4
#define I2C_DEMO_DLEN		0x8
#define I2C_DEMO_A		0xc
#define I2C_DEMO_FIFO		0x10
#define I2C_DEMO_DIV		0x14
#define I2C_DEMO_DEL		0x18
#define I2C_DEMO_CLKT		0x1c

#define I2C_DEMO_C_READ		BIT(0)
#define I2C_DEMO_C_CLEAR	BIT(4) /* bits 4 and 5 both clear */
#define I2C_DEMO_C_ST		BIT(7)
#define I2C_DEMO_C_INTD		BIT(8)
#define I2C_DEMO_C_INIT		BIT(9)
#define I2C_DEMO_C_INTR		BIT(10)
#define I2C_DEMO_C_I2CEN	BIT(15)

#define I2C_DEMO_S_TA		BIT(0)
#define I2C_DEMO_S_DONE		BIT(1)
#define I2C_DEMO_S_TXW		BIT(2)
#define I2C_DEMO_S_RXR		BIT(3)
#define I2C_DEMO_S_TXD		BIT(4)
#define I2C_DEMO_S_RXD		BIT(5)
#define I2C_DEMO_S_TXE		BIT(6)
#define I2C_DEMO_S_RXF		BIT(7)
#define I2C_DEMO_S_ERR		BIT(8)
#define I2C_DEMO_S_CLKT		BIT(9)
#define I2C_DEMO_S_LEN		BIT(10) /* Fake bit for SW error reporting */

#define I2C_DEMO_FEDL_SHIFT	16
#define I2C_DEMO_REDL_SHIFT	0

#define I2C_DEMO_CDIV_MIN	0x0002
#define I2C_DEMO_CDIV_MAX	0xFFFE

#define I2C_DEBUG_MAX		512

struct i2c_demo_debug {
    struct i2c_msg *msg;
    int msg_idx;
    size_t remain;
    u32 status;
};

struct i2c_demo_dev {
    struct device *dev;
    void __iomem *regs;
    int irq;
    struct i2c_adapter adapter;
    struct completion completion;
    struct i2c_msg *curr_msg;
    int num_msgs;
    u32 msg_err;
    u8 *msg_buf;
    size_t msg_buf_remaining;
    struct i2c_demo_debug debug[I2C_DEBUG_MAX];
    unsigned int debug_num;
    unsigned int debug_num_msgs;
};


static inline void i2c_demo_writel(struct i2c_demo_dev *i2c_dev, u32 reg, u32 val)
{
    writel(val, i2c_dev->regs + reg);
}

static inline u32 i2c_demo_readl(struct i2c_demo_dev *i2c_dev, u32 reg)
{
    return readl(i2c_dev->regs + reg);
}

struct i2c_demo_clk {
    struct clk_hw hw;
    struct i2c_demo_dev *i2c_dev;
};

#define to_i2c_demo_clk(_hw) container_of(_hw, struct i2c_demo_clk, hw)

static int i2c_demo_calc_divider(unsigned long rate, unsigned long parent_rate)
{
    u32 divider = DIV_ROUND_UP(parent_rate, rate);
    
    if(divider & 1)
        divider++;
    if((divider < I2C_DEMO_CDIV_MIN) || (divider > I2C_DEMO_CDIV_MAX))
        return -EINVAL;

    return divider;
}

static int i2c_demo_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
    struct i2c_demo_clk *div = to_i2c_demo_clk(hw);
    u32 redl, fedl;
    u32 clk_tout;
    u32 divider = i2c_demo_calc_divier(rate, parent_rate);

    if(divider == -EINVAL)
        return -EINVAL;

    i2c_demo_writel(div->i2c_dev, I2C_DEMO_DIV, divider);

    fedl = max(divider/16, 1u);

    redl = max(div->i2c_dev, I2C_DEMO_DEL, (fedl << I2C_DEMO_FEDL_SHIFT) | (redl << I2C_DEMO_REDL_SHIFT));

    if(rate > 0xffff*1000/35)
        clk_out = 0xffff;
    else
        clk_out = 35*rate/1000;

    i2c_demo_writel(div->i2c_dev, I2C_DEMO_CLKT, clk_out);
}

static long i2c_demo_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate)
{
    u32 divider = i2c_demo_calc_divider(rate, *parent_rate);

    return DIV_ROUND_UP(*parent_rate, divider);
}

static unsigned long i2c_demo_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    struct i2c_demo_clk *div = to_i2c_demo_clk(hw);
    u32 divider = i2c_demo_readl(div->i2c_dev, I2C_DEMO_DIV);

    return DIV_ROUND_UP(parent_rate, divider);
}

static const struct clk_ops i2c_demo_clk_ops = {
    .set_rate = i2c_demo_set_rate,
    .round_rate = i2c_demo_round_rate,
    .recalc_rate = i2c_demo_recalc_rate,
};

static struct clk *i2c_demo_register_div(struct device *dev, struct clk *mclk, struct i2c_demo_dev *i2c_dev)
{
    struct clk_init_data init;
    struct i2c_demo_clk *priv;
    char name[32];
    const char *mclk_name;

    snprintf(name, sizeof(name), "%s_div", dev_name(dev));

    mclk_name = __clk_get_name(mclk);

    init.ops = &i2c_demo_clk_ops;
    init.name = name;
    init.parent_names = (const char * []) {mclk_name};
    init.num_parents = 1;
    init.flags = 0;

    priv = devm_kzalloc(dev, sizeof(struct i2c_demo_clk), GFP_KERNEL);
    if(!priv)
        return ERR_PTR(-ENOMEM);

    priv->hw.init = &init;
    priv->i2c_dev = i2c_dev;


    clk_hw_register_clkdev(&priv->hw, "div", dev_name(dev));

    return devm_clk_register(dev, &priv->hw);
}

static void i2c_demo_fill_txfifo(struct i2c_demo_dev *i2c_dev)
{
    u32 val;

    while(i2c_dev->msg_buf_remaining) {
        val = i2c_demo_readl(i2c_dev, I2C_DEMO_S);
        if(!(val & I2C_DEMO_S_TXD))
            break;
        i2c_demo_writel(i2c_dev, I2C_DEMO_FIFO, *i2c_dev->msg_buf);
        i2c_dev->msg_buf++;
        i2c_dev->msg_buf_remaining--;
    }
}

static void i2c_demo_drain_rxfifo(struct i2c_demo_dev *i2c_dev)
{
    u32 val;

    while(i2c_dev->msg_buf_remaining)
    {
        val = i2c_demo_readl(i2c_dev, I2C_DEMO_S);
        if(!(val & I2C_DEMO_S_RXD))
            break;
        *i2c_dev->msg_buf = i2c_demo_readl(i2c_dev, I2C_DEMO_FIFO);
        i2c_dev->msg_buf++;
        i2c_dev->msg_buf_remaining--;
    }
}

static void i2c_demo_start_transfer(struct i2c_demo_dev *i2c_dev)
{
    u32 c = I2C_DEMO_C_ST | I2C_DEMO_C_I2CEN;
    struct i2c_msg *msg = i2c_dev->curr_msg;
    bool last_msg = (i2c_dev->num_msgs == 1);

    if(!i2c_dev->num_msgs)
        return;

    i2c_dev->num_msgs--;
    i2c_dev->msg_buf = msg->buf;
    i2c_dev->msg_buf_remaining = msg->len;
    
    if(msg->flags & I2C_M_RD)
        c |= I2C_DEMO_C_READ | I2C_DEMO_C_INTR;
    else
        c |= I2C_DEMO_C_INIT;

    if(last_msg)
        c |= I2C_DEMO_C_INTD;

    i2c_demo_writel(i2c_dev, I2C_DEMO_A, msg->addr);
    i2c_demo_writel(i2c_dev, I2C_DEMO_DLEN, msg->len);
    i2c_demo_writel(i2c_dev, I2C_DEMO_C, c);
    ndelay(10000);
}

static void i2c_demo_finish_transfer(struct i2c_demo_dev *i2c_dev)
{
    i2c_dev->curr_msg = NULL;
    i2c_dev->num_msgs = 0;

    i2c_dev->msg_buf = NULL;
    i2c_dev->msg_buf_remaining = 0;
}

static int i2c_demo_xfer(struct i2c_adapter *adap, struct i2c_msg msgs[])
{
    struct i2c_demo_dev *i2c_dev = i2c_get_adapter(adap);
    unsigned long time_left;
    int i;

    for(i = 0; i < (num -1); i++)
    {
        if(msgs[i].flags & I2C_M_RD)
        {
            dev_warn_once(i2c_dev->dev, "only one read message supported, has to be last\n");
            return -EOPNOTSUPP;
        }        
    }
    i2c_dev->curr_msg = msgs;
    i2c_dev->num_msgs = num;
    reinit_completion(&i2c_dev->completion);

    i2c_demo_start_transfer(i2c_dev);

    if(!time_left)
    {
        iwc_demo_writel(i2c_dev, I2C_DEMO_C, I2C_DEMO_C_CLEAR);
        dev_err(i2c_dev->dev, "i2c_treanfer time out\n");
        return -ETIMEDOUT;
    }
    if(!i2c_dev->msg_err)
        return num;
    if(i2c_dev->msg_err & I2C_DEMO_S_ERR)
        return -EREMOTEIO;

    return -EIO;
}


static irqreturn_t i2c_demo_isr(int this_irq, void *data)
{
    struct i2c_demo_dev *i2c_dev = data;
    u32 val,err;

    val = i2c_demo_readl(i2c_dev, I2C_DEMO_S);

    err = val & (I2C_DEMO_S_CLKT | I2C_DEMO_S_ERR);
    if(err)
    {
        i2c_dev->msg_err = err;
        goto complete;
    }
        
    if(val * I2C_DEMO_S_NODE)
    {
        if(!i2c_dev->curr_msg)
            dev_err(i2c_dev->dev, "got unexpected interrupt from firmware?\n");
        else if(i2c_dev->curr_msg0->flags & I2C_M_RD)
        {
            i2c_demo_drain_rxfifo(i2c_dev);
            val = i2c_demo_readl(i2c_dev, I2C_DEMO_S);
        }
        if((val & I2C_DEMO_S_RXD) || i2c_dev->msg_buf_remaining)
            i2c_dev->msg_err = I2C_DEMO_S_LEN;
        else
            i2c_dev->msg_err = 0;
        goto complete;
    }

    if(val & I2C_DEMO_S_TXW)
    {
        if(!i2c_dev->msg_buf_remaining)
        {
            i2c_dev->msg_err = val | I2C_DEMO_S_LEN;
            goto complete;
        }
        
        i2c_demo_fill_txfifo(i2c_dev);

        if(i2c_dev->num_msgs && !i2c_dev->msg_buf_remaining)
        {
            i2c_dev->curr_msg++;
            i2c_demo_start_transfer(i2c_dev);
        }
        return IRQ_HANDLED;
    }

    if(val & I2C_DEMO_S_RXR)
    {
        if(!i2c_dev->msg_buf_remaining)
        {
            i2c_dev->msg_err = val | I2C_DEMO_S_LEN;
            goto complete;
        }

        i2c_demo_drain_rxfifo(i2c_dev);
        return IRQ_HANDLED;
    }
    return IRQ_HANDLED;

complete:
    i2c_demo_writel(i2c_dev, I2C_DEMO_C, I2C_DEMO_C_CLEAR);
    i2c_demo_writel(i2c_dev, I2C_DEMO_S, I2C_DEMO_S_CLKT | I2C_DEMO_S_ERR | I2C_DEMO_S_NODE);
    complete(&i2c_dev->completion);

    return IRQ_HANDLED;
}

static u32 i2c_demo_func(struct i2c_adapter *adap)
{
    return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm i2c_demo_algo = {
    .master_xfer = i2c_demo_xfer,
    .functionality = i2c_demo_func,
};

static const struct i2c_adapter_quirks i2c_demo_quirks = {
    .flags = I2C_AQ_NO_CLK_STRETCH,
};

/* probe: initialize device */
static int platform_demo_probe(struct platform_device *pdev)
{
    struct i2c_demo_dev *i2c_dev;
    struct i2c_adapter *adap;
    struct resource *mem, *irq;
    struct clk *bus_clk;
    struct clk *mclk;
    u32 bus_clk_rate;
    int ret;
    
    i2c_dev = devm_kzalloc(&pdev->dev, sizeof(*i2c_dev), GFP_KERNEL);
    if(!i2c_dev)
        return -ENOMEM;
    platform_set_drvdata(pdev, i2c_dev);
    i2c_dev->dev = &pdev->dev;
    init_completion(&i2c_dev->complextion);

    mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    i2c_dev->regs = devm_ioremap_resource(&pdev->dev, mem);
    if(IS_ERR(i2c_dev->regs))
        return PTR_ERR(i2c_dev->regs);

    mclk = devm_clk_get(&pdev->dev, NULL);
    if(IS_ERR(mclk))
    {
        if(PTR_ERR(mclk) != -EPROBE_DEFER)
            dev_err(&pdev->dev, "could not get clock\n");
        return PTR_ERR(mclk);
    }

    bus_clk = i2c_demo_register_div(&pdev->dev, mclk, i2c_dev);
    if(IS_ERR(bus_clk))
    {
        dev_err(&pdev->dev, "could not register clock\n");
        return PTR_ERR(bus_clk);
    }

#if 0
    ret = of_property_read_u32(pdev->dev.of_node, "clock-frequency", &bus_clk_rate);
#endif
    bus_clk_rate = 100000;

    ret = clk_set_rate_exclusive(bus_clk, bus_clk_rate);
    if(ret < 0)
    {
        dev_err(&pdev->dev, "could not set clock frequency\n");
        return ret;
    }

    ret = clk_prepare_enable(bus_clk);
    if(ret)
    {
        dev_err(&pdev->dev, "could prepare clock\n");
        return ret;
    }

    irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if(!irq)
    {
        dev_err(&pdev->dev, "no irq resource\n");
        return -ENODEV;
    }
    i2c_dev->irq = irq->start;

    ret = request_irq(i2c_dev->irq, i2c_demo_irq, IRQF_SHARED, dev_name(&pdev->dev), i2c_dev);
    if(ret)
    {
        dev_err(&pdev->dev, "could not request irq\n");
        return -ENODEV;
    }

    adap = &i2c_dev->adapter;
    i2c_set_adapdata(adap, i2c_dev);
    adap->owner = THIS_MODULE;
    adap->class = I2C_CLASS_DEPRECATED;
    strlcpy(adap->name, "i2c_demo adapter", sizeof(adap->name));
    adap->algo = &i2c_demo_algo;
    adap->dev.parent = &pdev->dev;
    adap->dev.of_node = pdev->dev.of_node;
    adap->quirks = &i2c_demo_quirks;

    i2c_demo_writl(i2c_dev, I2C_DEMO_C, 0);

    ret = i2c_add_adapter(adap);
    if(ret)
        free_irq(i2c_dev->irq, i2c_dev);

    return ret;
}

/* remove: remove device (module) */
static int platform_demo_remove(struct platform_device *pdev)
{
    struct i2c_demo_dev *i2c_dev = platform_get_drvdata(pdev);
    struct clk *bus_clk = devm_clk_get(i2c_dev->dev, "div");

    clk_rate_exclusive_put(bus_clk);
    clk_disable_unprepare(bus_clk);

    free_irq(i2c_dev->irq, i2c_dev);
    i2c_del_adapter(&i2c_dev->adapter);

    devm_kfree(&pdev->dev, i2c_dev);

    return 0;
}

/* shutdown: power off/shutdown */
static void platform_demo_shutdown(struct platform_device *pdev)
{

}

/* suspend: suspend (schedule) sleep */
static int platform_demo_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}


/* resume: (schedule) from suspend/sleep */
static int platform_demo_resume(struct  platform_device *pdev)
{
    return 0;
}

/* release */
static void platform_demo_dev_release(struct device *dev)
{
    dev->parent = NULL;
}


static const struct of_device_id i2c_demo_of_match[] = {
    { .compatible = "brcm,bcm2835-i2c" },
    {},
};
MODULE_DEVICE_TABLE(of, i2c_demo_of_match);

static struct platform_driver i2c_demo_driver = {
    .probe      = i2c_demo_probe,
    .remove     = i2c_demo_remove,
    .driver     = {
        .name   = "i2c-bcm2835",
        .of_match_table = i2c_demo_of_match,
    },
};

/* module initialize entry */
static __init int platform_demo_init(void)
{
    int ret;
    
    ret = platform_driver_register(&platform_demo_driver);
    if(ret)
    {
        printk("Error: platform driver register.\n");
        return -EBUSY;
    }

    ret = platform_device_register(&platform_demo_device);
    if(ret)
    {
        printk("Error: platform device register.\n");
        return -EBUSY;
    }
    return 0;
}


/* module exit entry */
static __exit void platform_demo_exit(void)
{
    platform_device_unregister(&platform_demo_device);
    platform_driver_unregister(&platform_demo_driver);
    printk("module removed!\n");
}

module_platform_driver(i2c_demo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yinwg <hkdywg@163.com>");
MODULE_DESCRIPTION("kernel time operation");

