/*
 *  device_tree
 *  
 *  (C) 2021.04.25 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/init.h>
#include <linux/of_platform.h>
#include <linux/of_mdio.h>
#include <linux/mii.h>
#include <linux/of_net.h>
#include <linux/of_address.h>
#include <linux/errno.h>
#include <linux/phy.h>

static int cmd = 0;
module_param(cmd, int, 0644);
MODULE_PARM_DESC(cmd, "device test commond");

#define DEV_NAME                "DTS_demo"
#define RESOURCE_REG_NUM        (3)

#define DTS_CHILD_NODE          (0x0000)
#define DTS_MATCHING_NODE       (0x0001)
#define DTS_PROPERTY_ATTR       (0x0002)
#define DTS_FIND_OPERATION      (0x0004)
#define DTS_GET_OPERATION       (0x0008)
#define DTS_NODE_OPERATION      (0x0010)
#define DTS_PROPERTY_OPERATION  (0x0020)
#define DTS_ADDR_RESOURCE       (0x0040)

/* define neame for device and driever */
#define MII_PHY_ID              (0x02)
#define PHY_ID                  (0x01)
#define DTS_PHY_ID              (0x141)

/* fixup phy register on page 0 */
static unsigned short fixup_phy_copper_regs[] = {
    0x00,
    0x00,
    0x141,
    0x9c0,
    0x280,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

static const struct of_device_id dts_demo_of_match[] = {
    { .compatible = "DTS_demo, hkdywg", },
    { .compatible = "DTS_demo, hkdywg-X", },
    { .compatible = "virnode, hkdywg", },
};


/*
 * fixup mdio bus read
 *
 * @bus: mdio bus
 * @phy_addr: PHY device ID which range 0 to 31
 * @reg_num: Register address which range 0 to 31 on MDIO Clause 22
 *
 * Return special register value
 */
static int DTS_mdio_read(struct mii_bus *bus, int phy_addr, int reg_num)
{
    struct platform_device *pdev = bus->priv;
    struct device_node *np = pdev->dev.of_node;
    const phandle *ph;
    struct device_node *phy;
    int of_phy_id;

    /* find phy handle on cuurent device_node */
    ph = of_get_property(np, "phy-handle", NULL);
    /* find child node by handle */
    phy = of_find_node_by_phandle(be32_to_cpup(ph));
    if(!phy)
    {
        printk("unable to find device child node \n");
    }

    /* read phy id on mdio bus */
    of_property_read_u32(phy, "reg", &of_phy_id);
    if(of_phy_id < 0 || of_phy_id > 31)
        printk("invalid phy id from dt\n");

    return fixup_phy_copper_regs[reg_num];
}

/*
 * fixup mdio bus write
 *  @bus: mdio bus
 *  @phy_addr: phy device id which range 0 to 31
 *  @reg_num: register address which range 0 to 31 on mdio clause 22.
 *  @value: value need to write
 *
 *  return 0 always
 */
static int DTS_mdio_write(struct mii_bus *bus, int phy_addr, int reg_num, uint16_t val)
{
    return 0;
}


static int DTS_mdio_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    struct mii_bus *bus;
    unsigned short phy_id;
    struct phy_device *phy;

    bus = mdiobus_alloc();
    if(!bus)
    {
        printk("unable to allocate memory to mii_bus.\n");
        return -EINVAL;
    }

    bus->name = "DTS_mdio";
    bus->read = &DTS_mdio_read; 
    bus->write = &DTS_mdio_write;
    snprintf(bus->id, MII_BUS_ID_SIZE, "%s-mii", dev_name(&pdev->dev));
    bus->parent = &pdev->dev;
    bus->priv = pdev;
    
    platform_set_drvdata(pdev, bus);

    /* register mdio bus by dt */
    of_mdiobus_register(bus, np);

    /* mdio read test */
    phy_id = bus->read(bus, PHY_ID, MII_PHY_ID);
    if(phy_id == DTS_PHY_ID)
    {
        phy = get_phy_device(bus, PHY_ID, 0);
        if(!phy)
            printk("unable to get phy device.\n");
        printk("phy device ID: %#x\n", phy->phy_id);
    }
}

static __init int devicetree_demo_probe(struct platform_device *pdev)
{
    struct device_node *node;
    struct device_node *child;
    const struct of_device_id *match;
    int count;

    switch(cmd)
    {
        case DTS_CHILD_NODE:
        {
            node = pdev->dev.of_node;
            /* count child number for current device node */
            count = of_get_child_count(node);
            printk("%s has %d children\n", node->name, count);

            printk("%s child:\n", node->name);
            for_each_child_of_node(node, child)
                printk("%s \n", child->name);

            printk("%s available child:\n", node->name);
            for_each_available_child_of_node(node, child)
                printk("%s \n", child->name);
        }
        break;

        case DTS_MATCHING_NODE:
        {
            /* find all device nodes via device node id */
            for_each_matching_node(node, dts_demo_of_match)
            {
                if(node)
                    printk("Matching %s\n", node->name);
            }

            for_each_matching_node_and_match(node, dts_demo_of_match, &match)
            {
                if(match)
                    printk("device_id compatible: %s\n", match->compatible);
            }

            for_each_node_with_property(node, "hkdywg")
            {
                if(node)
                    printk("Found %s\n", node->full_name);
            }
        }
        break;

        case DTS_ADDR_RESOURCE:
        {
            struct device *dev =  &pdev->dev;
            struct resource iomem[RESOURCE_REG_NUM];
            void __iomem *base[RESOURCE_REG_NUM];
            int err, idx;
            node = dev->of_node;

            for(idx = 0; idx < RESOURCE_REG_NUM; idx++)
            {
                err = of_address_to_resource(node, idx, &iomem[idx]);
                if(err)
                {
                    dev_err(dev, "%d could not get IO memory\n", idx);
                    return err;
                }

                base[idx] = devm_ioremap_resource(dev, &iomem[idx]);
                if(IS_ERR(base[idx]))
                {
                    dev_err(dev, "%d could not IO remap\n", idx);
                    return PTR_ERR(base[idx]);
                }
                printk("found : %#lx - %#lx\n", (unsigned long)iomem[idx].start, (unsigned long)iomem[idx].end);
            }
            for(idx = 0; idx < RESOURCE_REG_NUM; idx++)
            {
                devm_iounmap(dev, base[idx]);
            }
        }
        break;

        case DTS_PROPERTY_ATTR:
        {
            struct device_node *npp;
            struct property *prop;
            const char *comp;
            int err;
            node = pdev->dev.of_node;
    
            if(of_device_is_available(node))
                printk("%s device is avalible\n", node->name);

            /* find next device node */
            npp = of_find_all_nodes(node);
            if(!npp)
                printk("unable to get next device_node.\n");
            /* get property */
            prop = of_find_property(node, "compatible", NULL);

            /* read compatible property */
            comp = of_get_property(node, "compatible", NULL);

            /* compare compatible poperty value */
            if(of_compat_cmp(comp, "DTS_demo, hkdywg", strlen(comp)) == 0)
                printk("%s property exist\n", prop->name);

            /* compare node name */
            if(of_node_cmp(node->name, "DTS_demo") == 0)
                printk("%s node exist\n", node->name);

            err = of_count_phandle_with_args(node, "phy-handle", "#phy-cells");
            if(err < 0)
                printk("unable to parase phandle.\n");
        }
        break;

        case DTS_FIND_OPERATION:
        {
            struct platform_device *dev;
            const struct of_device_id *match_np;
            const char *comp = NULL;
            const phandle *ph;
            struct device_node *phy;
            const char *value;
            struct property *pp;
            int len;
            node = pdev->dev.of_node;

            /* find device node via compatible property */
            node = of_find_compatible_node(NULL, NULL, "DTS_demo, hkdywg");
            if(node)
                printk("found %s\n", node->full_name);

            dev = of_find_device_by_node(node);
            if(dev)
                printk("Platform device: %s\n", dev_name(&dev->dev));

            /* find a device node via device node id */
            node = of_find_matching_node(NULL, dts_demo_of_match);
            if(node)
                printk("Matching %s\n", node->name);

            node = of_find_matching_node_and_match(NULL, dts_demo_of_match, &match_np);
            if(match_np)
                printk("device node id: %s\n", match_np->compatible);

            /* find device by path */
            node = of_find_node_by_path("/DTS_demo");
            if(node)
            {
                of_property_read_string(node, "compatible", &comp);
                if(comp)
                    printk("%s compatible: %s\n",node->name, comp);
            }

            /* find a phandle on current device_node */
            node = pdev->dev.of_node;
            ph = of_get_property(node, "phy-hanlde", NULL);
            if(!ph)
                printk("unable to find 'phy-handle' on current device\n");

            phy = of_find_node_by_phandle(be32_to_cpup(ph));
            if(!phy)
                printk("unable to find device node: phy1\n");

            /* read property from special device node */
            value = of_get_property(phy, "compatible", NULL);
            if(value)
                printk("PHY0: %s\n", value);

            /* find device node via property name */
            node = of_find_node_with_property(NULL, "name-space");
            if(node)
                printk("found %s\n", node->full_name);

            /* find int property on current device-node */
            node = pdev->dev.of_node;
            pp = of_find_property(node, "viros_int", &len);
            printk("property: %s\n", pp->name);
            printk("value: %#x\n", of_read_number(pp->value, len/4));
            printk("length: %d\n", len);

            /* find string property on current device-node */
            pp = of_find_property(node, "viros_name", NULL);
            printk("property: %s . value: %s\n",pp->name, pp->value);
        }
        break;
        
        case DTS_GET_OPERATION:
        {
            const uint32_t *regaddr_p;
            uint64_t addr;
            struct device_node *child, *cpu_node, *parent;
            const struct of_device_id *match;
            const char *comp, *mac;
            const __be32 *prop;
            int mode;
            int len;
            int count;
            int cells;
            node =  pdev->dev.of_node;

            /* get first address from 'reg' property */
            regaddr_p = of_get_address(node, 0, &addr, NULL);
            if(regaddr_p)
                printk("%s's address[0]: %#llx\n", node->name, addr);

            /* get second address from 'reg' property */
            regaddr_p = of_get_address(node, 1, &addr, NULL);
            if(regaddr_p)
                printk("%s's address[1]: %#llx\n", node->name, addr);
            
            /* get child node by name */
            child = of_get_child_by_name(node, "child0");
            if(child)
                printk("%s child: %s\n",node->full_name, child->full_name);

            /* count child number for current device node */
            count  = of_get_child_count(node);
            printk("%s has %d children\n", node->name, count);

            for_each_child_of_node(node, child)
                printk("%s \n",child->name);

            for_each_available_child_of_node(node, child)
                printk("avalible child node : %s\n", child->name);

            /* read local cpu 0 */
            cpu_node = of_get_cpu_node(0, NULL);
            if(cpu_node)
            {
                printk("CPU0: %s\n", cpu_node->name);
                of_property_read_string(cpu_node, "compatible", &comp);
                if(comp)
                    printk("compatible: %s\n", comp);
            }

            /* abatain mac address from dts */
            mac = of_get_mac_address(node);
            if(mac)
                printk("MAC address: %02x %02x %02x %02x %02x %02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        
            /* find next child */
            child =  of_get_next_child(node, NULL);
            if(child)
                printk("%s child: %s\n",node->full_name, child->full_name);
    
            parent = of_get_parent(child);
            if(parent)
                printk("%s's parent: %s\n",child->full_name, parent->full_name);
        
            /* read phy mode from dts */
            mode = of_get_phy_mode(node);
            if(mode == PHY_INTERFACE_MODE_RGMII)
                printk("Phy mode: rgmii\n");

            /* abtain string property from device node */
            prop = of_get_property(node, "viros_name", &len);
            if(prop)
            {
                printk("property: viros_name is %s\n",  prop);
            }

            /* abatain int property from device node */
            prop = of_get_property(node, "viros_int", &len);
            if(prop)
            {
                int value;
                value  = be32_to_cpup(prop);
                printk("property: viros_int value is %#x\n",value);
            }

            /* abatain multi-int property from device  node */
            prop = of_get_property(node, "viros_mult", &len);
            if(prop)
            {
                unsigned long value;
                value  = of_read_ulong(prop, of_n_addr_cells(node));
                printk("#cell 0: %#llx\n", value);

                prop += of_n_addr_cells(node);
                value = of_read_ulong(prop, of_n_addr_cells(node));
                printk("#cell 1: %#llx\n", value);
            }

            /* abatain empty-value property on device node */
            prop = of_get_property(node, "viros_leg", NULL);
            if(prop)
                printk("viros_leg property exist.\n");

            /* get #address-cells numbers */
            cells = of_n_addr_cells(node);
            printk("%s #address-cells: %#x\n", node->name, cells);

            cells =of_n_size_cells(node);
            printk("%s #size-cells: %#x\n", node->name, cells);
        }
        break;

        case DTS_NODE_OPERATION:
        {
            struct property *prop;
            const char *comp;
            struct device_node *phy;
            struct phanlde *ph;
            struct of_phandle_args args;
            int ret, index;
            const char *value;
            node = pdev->dev.of_node;

            /* get property */
            prop = of_find_property(node, "compatible", NULL);

            /* read compatible property */
            comp = of_get_property(node, "compatible", NULL);

            /* compare compatible property value */
            if(of_compat_cmp(comp, "DTS_demo, hkdywg", strlen(comp)) == 0)
                printk("%s compatible: %s\n", node->name, comp);

            /* compare property name */
            if(of_prop_cmp(prop->name, "compatible") == 0)
                printk("%s property exist.\n", node->name);

            comp = of_node_full_name(node);
            printk("%s full name: %s\n", node->name, comp);

            /* check device node is root */
            if(of_node_is_root(node))
                printk("%s is root node.\n", node->name);
            else
                printk("%s is not root node.\n",node->name);

            phy = of_parse_phandle(node, "phy-hanlde", 0);
            if(!phy)
                printk("unable read phy-handle property. \n");

            /* read property from special device node */
            value = of_get_property(phy, "compatible", NULL);
            if(value)
                printk("PHY: %s\n", value);

            /* read gpio phandle argument */
            ret = of_parse_phandle_with_args(node, "reset-gpio", "#gpio-cells", 0, &args);
            if(ret < 0)
                printk("unable to parse gpio handle.\n");

            comp = of_get_property(args.np, "compatible", NULL);
            if(comp)
                printk("%s compatible: %s\n", args.np->name, comp);

            printk("GPIO index: %#x\n", args.args[0]);
            printk("GPIO egde: %#x\n", args.args[1]);
        }
        break;
    
        case DTS_PROPERTY_OPERATION:
        {
            struct device_node *child;
            struct property *prop;
            const char *lane = NULL;
            const __be32 *lane_32 = NULL;
            const char *string_array[4], *string;
            int  ret, count, index;
            uint32_t data, i;
            bool bool_data;
            uint8_t u8_data;
            uint16_t u16_data, u16_array[5];
            uint32_t u32_data, u32_array[5];
            const uint32_t *regaddr_p;
            uint64_t addr, regaddr;
            node = pdev->dev.of_node;

            prop = of_find_property(node, "viros-strings", NULL);
            for(lane = of_prop_next_string(prop, NULL); lane; lane = of_prop_next_string(prop, lane))
                printk("string value: %s\n", lane);

            prop = of_find_property(node, "viros-data", NULL);
            if(prop)
            {
                for(i = 0; i < 4; i++)
                {
                    lane_32 = of_prop_next_u32(prop, lane_32, &data);
                    printk("data[%d]: %#x\n", i, data);
                }
            }

            /* veriry property whether contains special string. */
            ret = of_property_match_string(node, "viros-strings", "rootfs");
            if(ret < 0)
                printk("viros-strings doesn't contain rootfs\n");
            else
                printk("viros-strings[%d]  contain rootfs\n", ret);

            /* count the string on string-list property */
            count = of_property_count_strings(node, "viros-strings");
            printk("strings count: %#x\n", count);

            /* read special string on string-list property with index */
            for(index = 0; index < count; index++)
            {
                ret = of_property_read_string_index(node, "viros-stings", index, &string);
                if(ret < 0)
                {
                    printk("unable to read viros-strings[%d]\n", index);
                    continue;
                }
                printk("viros-strings[%d]: %s\n", index, string);
            }

            /*read numer of strings form string-list property*/
            ret = of_property_read_string_array(node, "viros-strings", string_array, count);
            for(index = 0; index < count; index++)
                printk("string_array[%d]: %s\n", index, string_array[index]);
            
            of_property_for_each_string(node, "viros-strings", prop, lane)
                printk("String value: %s\n", lane);

            of_property_for_each_u32(node, "viros-data", prop, lane_32, data);
                printk("Data[%d]: %#x\n", i++, data);

            /* read bool data from property */
            bool_data = of_property_read_bool(node, "viros-data");
            printk("bool_data: %#x\n", bool_data);

            /* read u8 data from property */
            of_property_read_u8(node, "viros-data", &u8_data);
            printk("u8_data:    %x\n", u8_data);
            /* read u16 data from property */
            of_property_read_u16(node, "viros-data", &u16_data);
            printk("u16_data:    %x\n", u16_data);
            /* read u32 data from property */
            of_property_read_u32(node, "viros-data", &u32_data);
            printk("u32_data:    %x\n", u32_data);

            /* read array from device_node */
            ret = of_property_read_u16_array(node, "viros-data-array", u16_array, 5);
            if(ret != 0)
                printk("unable to read array from device node\n");
            else
                printk("Array: %x %x %x %x %x %x\n", u16_array[0], u16_array[1], u16_array[2], u16_array[3], u16_array[4]);
            
            /* read array from device_node */
            ret = of_property_read_u32_array(node, "viros-data-array", u32_array, 5);
            if(ret != 0)
                printk("unable to read array from device node\n");
            else
                printk("Array: %x %x %x %x %x %x\n", u32_array[0], u32_array[1], u32_array[2], u32_array[3], u32_array[4]);
    
            /* get property */
            lane_32 = of_get_property(node, "viros-data", NULL);

            /* read lst-ulong data */
            u32_data = of_read_ulong(lane_32, 1);
            printk("viros-data 1st u32 value: %x\n", u32_data);

            /* read 2nd-ulong data */
            u32_data = of_read_ulong(lane_32, 2);
            printk("viros-data 2nd u32 value: %x\n", u32_data);

            /* get child device node which named "child0" */
            child = of_get_child_by_name(node, "child0");

            /* get first address from 'reg' property */
            regaddr_p = of_get_address(child, 0, &addr, NULL);
            if(regaddr_p)
                printk("%s's address[0]: %llx\n", child->name, addr);

            /* translate address to physical address */
            regaddr = of_translate_address(child, regaddr_p);
            if(regaddr)
                printk("%s's physical address: %llx\n", child->name, regaddr);
        }
        break;


       default:
            printk("task: do nothing\n");
        break;
    }

    return 0;
}

static __exit int devicetree_demo_remove(struct platform_device *pdev)
{
    printk("module removed!\n");
    return 0;
}


/* platform driver information */
static struct platform_driver dts_demo_driver = {
    .probe = devicetree_demo_probe,
    .remove = devicetree_demo_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = DEV_NAME,
        .of_match_table = dts_demo_of_match,
    },
};

module_platform_driver(dts_demo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yinwg <hkdywg@163.com>");
MODULE_DESCRIPTION("kernel time operation");

