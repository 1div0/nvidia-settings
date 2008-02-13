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

#include "ctkui.h"
#include "ctkwindow.h"

#include <gtk/gtk.h>

/*
 * This source file provides thin wrappers over the gtk routines, so
 * that nvidia-settings.c doesn't need to include gtk+
 */

void ctk_init(int *argc, char **argv[])
{
    gtk_init(argc, argv);
}

char *ctk_get_display(void)
{
    return gdk_get_display();
}

void ctk_main(NvCtrlAttributeHandle **screen_handles, int num_screen_handles,
              NvCtrlAttributeHandle **gpu_handles, int num_gpu_handles,
              NvCtrlAttributeHandle **vcsc_handles, int num_vcsc_handles,
              ParsedAttribute *p, ConfigProperties *conf)
{
    int i, has_nv_control = FALSE;
    
    ctk_window_new(screen_handles, num_screen_handles,
                   gpu_handles, num_gpu_handles,
                   vcsc_handles, num_vcsc_handles,
                   p, conf);
    
    for (i = 0; i < num_screen_handles; i++) {
        if (screen_handles[i]) {
            has_nv_control = TRUE;
            break;
        }
    }
    
    if (!has_nv_control) {
        GtkWidget *dlg;
        dlg = gtk_message_dialog_new (NULL,
                                      GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING,
                                      GTK_BUTTONS_OK,
                                      "You do not appear to be using the NVIDIA "
                                      "X driver. Please edit your X configuration "
                                      "file (just run `nvidia-xconfig` "
                                      "as root), and restart the X server. ");
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy (dlg);
    }

    gtk_main();
}
