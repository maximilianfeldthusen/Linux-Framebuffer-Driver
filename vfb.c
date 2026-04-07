
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>

#define FB_NAME "vfb"

/* Default Resolution and Color Depth */
static int xres = 800;
static int yres = 600;
static int bpp = 32;

module_param(xres, int, 0644);
module_param(yres, int, 0644);
module_param(bpp, int, 0644);
MODULE_PARM_DESC(xres, "Horizontal resolution");
MODULE_PARM_DESC(yres, "Vertical resolution");
MODULE_PARM_DESC(bpp, "Bits per pixel (16, 24, or 32)");

static struct fb_info *vfb_info;
static u32 *pseudo_palette;

/* Validate screen variable changes */
static int vfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
    if (var->xres > xres || var->yres > yres)
        return -EINVAL;
    
    if (var->bits_per_pixel != bpp)
        return -EINVAL;

    /* Ensure standard timing */
    var->xres_virtual = var->xres;
    var->yres_virtual = var->yres;
    var->xoffset = 0;
    var->yoffset = 0;

    return 0;
}

static int vfb_setcolreg(unsigned regno, unsigned red, unsigned green,
                         unsigned blue, unsigned transp, struct fb_info *info)
{
    if (regno < 16 && pseudo_palette) {
        /* Convert 0-255 to 0-65535 range for hardware palette */
        u32 val = ((red & 0xff00) << 8) | ((green & 0xff00) << 0) |
                  ((blue & 0xff00) >> 8);
        pseudo_palette[regno] = val;
        return 0;
    }
    return -EINVAL;
}

/* Basic blanking support for power management */
static int vfb_blank(int blank_mode, struct fb_info *info)
{
    switch (blank_mode) {
    case FB_BLANK_UNBLANK:
    case FB_BLANK_NORMAL:
        /* In a real driver, you might turn off the backlight here */
        break;
    case FB_BLANK_POWERDOWN:
        /* Clear screen on power down */
        memset(info->screen_base, 0, info->screen_size);
        break;
    }
    return 0;
}

static struct fb_ops vfb_ops = {
    .owner = THIS_MODULE,
    .fb_read = fb_sys_read,
    .fb_write = fb_sys_write,
    .fb_fillrect = sys_fillrect,
    .fb_copyarea = sys_copyarea,
    .fb_imageblit = sys_imageblit,
    .fb_setcolreg = vfb_setcolreg,
    .fb_check_var = vfb_check_var,
    .fb_blank = vfb_blank,
};

static int __init vfb_init(void)
{
    int size;
    int ret;

    /* Validate parameters */
    if (xres <= 0 || yres <= 0 || bpp <= 0) {
        pr_err("Invalid resolution or bits-per-pixel parameters\n");
        return -EINVAL;
    }

    /* Calculate buffer size */
    size = xres * yres * bpp / 8;

    /* Allocate framebuffer info structure */
    vfb_info = framebuffer_alloc(0, NULL);
    if (!vfb_info) {
        pr_err("Failed to allocate framebuffer info\n");
        return -ENOMEM;
    }

    /* Allocate video memory */
    vfb_info->screen_base = vzalloc(size);
    if (!vfb_info->screen_base) {
        pr_err("Failed to allocate video memory\n");
        ret = -ENOMEM;
        goto err_free_fb;
    }
    vfb_info->screen_size = size;

    /* Allocate palette memory (even if unused in truecolor, good practice) */
    pseudo_palette = kmalloc(16 * sizeof(u32), GFP_KERNEL);
    if (!pseudo_palette) {
        pr_err("Failed to allocate palette memory\n");
        ret = -ENOMEM;
        goto err_free_mem;
    }

    /* Initialize ops */
    vfb_info->fbops = &vfb_ops;
    vfb_info->flags = FBINFO_DEFAULT;

    /* Set fixed screen info */
    vfb_info->fix = (struct fb_fix_screeninfo){
        .id = FB_NAME,
        .type = FB_TYPE_PACKED_PIXELS,
        .visual = (bpp == 8) ? FB_VISUAL_PSEUDOCOLOR : FB_VISUAL_TRUECOLOR,
        .line_length = xres * bpp / 8,
        .smem_len = size,
    };

    /* Set variable screen info */
    vfb_info->var = (struct fb_var_screeninfo){
        .xres = xres,
        .yres = yres,
        .xres_virtual = xres,
        .yres_virtual = yres,
        .bits_per_pixel = bpp,
        .red = {16, 8, 0},
        .green = {8, 8, 0},
        .blue = {0, 8, 0},
        .transp = {24, 8, 0},
        .activate = FB_ACTIVATE_NOW,
        .height = -1,
        .width = -1,
        .grayscale = 0,
    };

    vfb_info->pseudo_palette = pseudo_palette;

    /* Register the framebuffer */
    ret = register_framebuffer(vfb_info);
    if (ret < 0) {
        pr_err("Failed to register framebuffer: %d\n", ret);
        goto err_free_pal;
    }

    pr_info("Virtual framebuffer device registered (%dx%d@%dbpp)\n", xres, yres, bpp);
    return 0;

err_free_pal:
    kfree(pseudo_palette);
err_free_mem:
    vfree(vfb_info->screen_base);
err_free_fb:
    framebuffer_release(vfb_info);
    return ret;
}

static void __exit vfb_exit(void)
{
    if (vfb_info) {
        unregister_framebuffer(vfb_info);
        
        if (vfb_info->screen_base)
            vfree(vfb_info->screen_base);
            
        if (pseudo_palette)
            kfree(pseudo_palette);
            
        framebuffer_release(vfb_info);
        pr_info("Virtual framebuffer device unregistered\n");
    }
}

module_init(vfb_init);
module_exit(vfb_exit);

MODULE_LICENSE("MIT");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Advanced Virtual Framebuffer Driver with configurable resolution");
