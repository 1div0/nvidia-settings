/*
 * nvidia-settings: A tool for configuring the NVIDIA X driver on Unix
 * and Linux systems.
 *
 * Copyright (C) 2006 NVIDIA Corporation.
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

#ifndef __CTK_VCSC_H__
#define __CTK_VCSC_H__

#include "ctkevent.h"
#include "ctkconfig.h"

G_BEGIN_DECLS

#define CTK_TYPE_VCSC (ctk_vcsc_get_type())

#define CTK_VCSC(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_TYPE_VCSC, \
                                 CtkVcsc))

#define CTK_VCSC_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST ((klass), CTK_TYPE_VCSC, \
                              CtkVcscClass))

#define CTK_IS_VCSC(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_TYPE_VCSC))

#define CTK_IS_VCSC_CLASS(class) \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_TYPE_VCSC))

#define CTK_VCSC_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_TYPE_VCSC, \
                                CtkVcscClass))


typedef struct _CtkVcsc
{
    GtkVBox parent;

    CtkConfig *ctk_config;
    NvCtrlAttributeHandle *handle;

} CtkVcsc;

typedef struct _CtkVcscClass
{
    GtkVBoxClass parent_class;
} CtkVcscClass;


GType       ctk_vcsc_get_type  (void) G_GNUC_CONST;
GtkWidget*  ctk_vcsc_new       (NvCtrlAttributeHandle *, CtkConfig *);

GtkTextBuffer *ctk_vcsc_create_help(GtkTextTagTable *,
                                    CtkVcsc *);

G_END_DECLS

#endif /* __CTK_VCSC_H__ */
