/*
 * nvidia-settings: A tool for configuring the NVIDIA X driver on Unix
 * and Linux systems.
 *
 * Copyright (C) 2004 NVIDIA Corporation.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of Version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See Version 2
 * of the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the:
 *
 *           Free Software Foundation, Inc.
 *           59 Temple Place - Suite 330
 *           Boston, MA 02111-1307, USA
 *
 */

#include <gtk/gtk.h>
#include "NvCtrlAttributes.h"

#include <stdio.h>

#include "ctkimage.h"

#include "ctkdevice.h"
#include "ctkhelp.h"

static void add_table_row(GtkWidget *table,
                          const gint row,
                          const gint value_alignment,
                          const gchar *name,
                          const gchar *value);

#define N_GDK_PIXBUFS 45


GType ctk_device_get_type(
    void
)
{
    static GType ctk_device_type = 0;

    if (!ctk_device_type) {
        static const GTypeInfo info_ctk_device = {
            sizeof (CtkDeviceClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            NULL, /* class_init */
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (CtkDevice),
            0, /* n_preallocs */
            NULL, /* instance_init */
        };

        ctk_device_type =
            g_type_register_static(GTK_TYPE_VBOX,
                                   "CtkDevice", &info_ctk_device, 0);
    }
    
    return ctk_device_type;
}



GtkWidget* ctk_device_new(
    NvCtrlAttributeHandle *handle
)
{
    GObject *object;
    CtkDevice *ctk_device;
    GtkWidget *label;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *banner;
    GtkWidget *hseparator;
    GtkWidget *table;
    GtkWidget *alignment;

    guint8 *image_buffer = NULL;
    const nv_image_t *img;

    char *product_name, *bus_type, *vbios_version, *video_ram, *irq;
    char *os, *arch, *version;
    char scratch[64];
    ReturnStatus ret;
    gint tmp, os_val;

    gchar *__unknown = "Unknown";

    /*
     * get the data that we will display below
     * 
     * XXX should be able to update any of this if an attribute
     * changes.
     */

    /* NV_CTRL_STRING_PRODUCT_NAME */

    ret = NvCtrlGetStringAttribute(handle, NV_CTRL_STRING_PRODUCT_NAME,
                                   &product_name);
    if (ret != NvCtrlSuccess) product_name = "Unknown GPU";
    
    /* NV_CTRL_BUS_TYPE */

    ret = NvCtrlGetAttribute(handle, NV_CTRL_BUS_TYPE, &tmp);
    bus_type = NULL;
    if (ret == NvCtrlSuccess) {
        if (tmp == NV_CTRL_BUS_TYPE_AGP) bus_type = "AGP";
        if (tmp == NV_CTRL_BUS_TYPE_PCI) bus_type = "PCI";
        if (tmp == NV_CTRL_BUS_TYPE_PCI_EXPRESS) bus_type = "PCI Express";
    }
    if (!bus_type) bus_type = __unknown;
    
    /* NV_CTRL_STRING_VBIOS_VERSION */

    ret = NvCtrlGetStringAttribute(handle, NV_CTRL_STRING_VBIOS_VERSION,
                                   &vbios_version);
    if (ret != NvCtrlSuccess) vbios_version = __unknown;
    
    /* NV_CTRL_VIDEO_RAM */

    ret = NvCtrlGetAttribute(handle, NV_CTRL_VIDEO_RAM, &tmp);
    if (ret != NvCtrlSuccess) tmp = 0;
    video_ram = g_strdup_printf("%d MB", tmp >> 10);
        
    /* NV_CTRL_IRQ */
    
    ret = NvCtrlGetAttribute(handle, NV_CTRL_IRQ, &tmp);
    if (ret != NvCtrlSuccess) tmp = 0;
    irq = g_strdup_printf("%d", tmp);
    
    /* NV_CTRL_OPERATING_SYSTEM */

    os_val = NV_CTRL_OPERATING_SYSTEM_LINUX;
    ret = NvCtrlGetAttribute(handle, NV_CTRL_OPERATING_SYSTEM, &os_val);
    os = NULL;
    if (ret == NvCtrlSuccess) {
        if (os_val == NV_CTRL_OPERATING_SYSTEM_LINUX) os = "Linux";
        else if (os_val == NV_CTRL_OPERATING_SYSTEM_FREEBSD) os = "FreeBSD";
        else if (os_val == NV_CTRL_OPERATING_SYSTEM_SUNOS) os = "SunOS";
    }
    if (!os) os = __unknown;

    /* NV_CTRL_ARCHITECTURE */
    
    ret = NvCtrlGetAttribute(handle, NV_CTRL_ARCHITECTURE, &tmp);
    arch = NULL;
    if (ret == NvCtrlSuccess) {
        if (tmp == NV_CTRL_ARCHITECTURE_X86) arch = "x86";
        if (tmp == NV_CTRL_ARCHITECTURE_X86_64) arch = "amd64";
        if (tmp == NV_CTRL_ARCHITECTURE_IA64) arch = "ia64";
    }
    if (!arch) arch = __unknown;

    snprintf(scratch, 64, "%s-%s", os, arch);

    /* NV_CTRL_STRING_NVIDIA_DRIVER_VERSION */

    ret = NvCtrlGetStringAttribute(handle,
                                   NV_CTRL_STRING_NVIDIA_DRIVER_VERSION,
                                   &version);
    if (ret != NvCtrlSuccess) version = __unknown;
    
    
    
    /* now, create the object */
    
    object = g_object_new(CTK_TYPE_DEVICE, NULL);
    ctk_device = CTK_DEVICE(object);

    /* cache the attribute handle */

    ctk_device->handle = handle;

    /* set container properties of the object */

    gtk_box_set_spacing(GTK_BOX(ctk_device), 10);

    /* banner */

    if (os_val == NV_CTRL_OPERATING_SYSTEM_LINUX) {
        banner = ctk_banner_image_new(BANNER_ARTWORK_PENGUIN);
    } else if (os_val == NV_CTRL_OPERATING_SYSTEM_FREEBSD) {
        banner = ctk_banner_image_new(BANNER_ARTWORK_BSD);
    } else if (os_val == NV_CTRL_OPERATING_SYSTEM_SUNOS) {
        banner = ctk_banner_image_new(BANNER_ARTWORK_SOLARIS);
    } else {
        banner = ctk_banner_image_new(BANNER_ARTWORK_PENGUIN);
    }
    gtk_box_pack_start(GTK_BOX(ctk_device), banner, FALSE, FALSE, 0);
        
    /*
     * Device information: TOP->MIDDLE - LEFT->RIGHT
     *
     * This displays basic display adatper information, including
     * product name, bios version, bus type, video ram and interrupt
     * line.
     */

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(ctk_device), vbox, TRUE, TRUE, 0);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    label = gtk_label_new("Graphics Card Information");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    hseparator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(hbox), hseparator, TRUE, TRUE, 5);

    table = gtk_table_new(7, 2, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 0);

    gtk_table_set_row_spacings(GTK_TABLE(table), 3);
    gtk_table_set_col_spacings(GTK_TABLE(table), 15);

    gtk_container_set_border_width(GTK_CONTAINER(table), 5);

    
    add_table_row(table, 0, 0, "Graphics Processor:", product_name);
    add_table_row(table, 1, 0, "Bus Type:", bus_type);
    add_table_row(table, 2, 0, "VBIOS Version:", vbios_version);
    add_table_row(table, 3, 0, "Video Memory:", video_ram);
    add_table_row(table, 4, 0, "IRQ:", irq);
    add_table_row(table, 5, 0, "Operating System:", scratch);
    add_table_row(table, 6, 0, "NVIDIA Driver Version:", version);

  
   
    gtk_widget_show_all(GTK_WIDGET(object));
    
    return GTK_WIDGET(object);
}


static void add_table_row(GtkWidget *table,
                          const gint row,
                          const gint value_alignment,
                          const gchar *name,
                          const gchar *value)
{
    GtkWidget *label;

    label = gtk_label_new(name);
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, row, row + 1,
                     GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

    label = gtk_label_new(value);
    gtk_misc_set_alignment(GTK_MISC(label), value_alignment, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 1, 2, row, row + 1,
                     GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
}
    
    
GtkTextBuffer *ctk_device_create_help(GtkTextTagTable *table,
                                      const gchar *screen_name)
{
    GtkTextIter i;
    GtkTextBuffer *b;

    b = gtk_text_buffer_new(table);
    
    gtk_text_buffer_get_iter_at_offset(b, &i, 0);

    ctk_help_title(b, &i, "Graphics Card Information Help");

    ctk_help_para(b, &i, "This page in the NVIDIA "
                  "X Server Control Panel describes basic "
                  "information about the Graphics Processing Unit "
                  "(GPU) on which the X screen '%s' is running.",
                  screen_name);
    
    ctk_help_heading(b, &i, "Graphics Processor");
    ctk_help_para(b, &i, "This is the product name of the GPU.");
    
    ctk_help_heading(b, &i, "Bus Type");
    ctk_help_para(b, &i, "This is the bus type which is "
                  "used to connect the NVIDIA GPU to the rest of "
                  "your computer; possible values are AGP, PCI, or "
                  "PCI Express.");
    
    ctk_help_heading(b, &i, "VBIOS Version");
    ctk_help_para(b, &i, "This is the Video BIOS version.");
    
    
    ctk_help_heading(b, &i, "Video Memory"); 
    ctk_help_para(b, &i, "This is the amount of video memory on your "
                  "graphics card.");

    ctk_help_heading(b, &i, "IRQ");
    ctk_help_para(b, &i, "This is the interrupt request line assigned to "
                  "this GPU.");
    
    ctk_help_heading(b, &i, "Operating System");
    ctk_help_para(b, &i, "This is the operating system on which the NVIDIA "
                  "X driver is running; possible values are "
                  "'Linux' and 'FreeBSD'.  This also specifies the platform "
                  "on which the operating system is running, such as x86, "
                  "amd64, or ia64");
    
    ctk_help_heading(b, &i, "NVIDIA Driver Version");
    ctk_help_para(b, &i, "This is the version of the NVIDIA Accelerated "
                  "Graphics Driver currently in use.");
    
    ctk_help_finish(b);

    return b;
}
