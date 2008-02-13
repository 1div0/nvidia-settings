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
#include "ctkutils.h"
                                                                                            
#include <stdlib.h>

 
void add_table_row(GtkWidget *table,
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

    if (value == NULL)
        label = gtk_label_new("Unknown");
    else
        label = gtk_label_new(value);
    gtk_misc_set_alignment(GTK_MISC(label), value_alignment, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 1, 2, row, row + 1,
                     GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
}
    
