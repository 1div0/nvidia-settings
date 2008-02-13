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

/*
 * The CtkConfig widget controls configuration options of the control
 * panel itself (rather than configuration options of the NVIDIA X/GLX
 * driver).
 */

#include "ctkconfig.h"
#include "ctkhelp.h"

#include "configuration_banner.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

static const char *__tooltip_help =
"When ToolTips are enabled, descriptions will be displayed next to options "
"when the mouse is held over them.";

static const char *__status_bar_help =
"The status bar in the bottom "
"right of the nvidia-settings GUI displays the most "
"recent change that has been sent to the X "
"server.  The 'Display Status Bar' check box "
"controls whether this status bar is displayed.";

static const char *__slider_text_entries_help =
"When the \"Slider Text Entries\" option is enabled, the current "
"value of an attribute controlled by a slider is "
"displayed and can be modified with a text entry "
"shown next to the slider.";

static const char *__x_display_names_help =
"When the current settings are saved to the "
"configuration file, the attributes can either be "
"qualified with just the screen to which the attribute "
"should be applied, or the attribute can be qualifed with "
"the entire X Display name.  If you want to be able to "
"use the same configuration file across multiple "
"computers, be sure to leave this option unchecked.  "
"It is normally recommended to leave this option "
"unchecked.";


static void ctk_config_class_init(CtkConfigClass *ctk_config_class);

static void display_status_bar_toggled(GtkWidget *, gpointer);
static void tooltips_toggled(GtkWidget *, gpointer);
static void slider_text_entries_toggled(GtkWidget *, gpointer);
static void display_name_toggled(GtkWidget *widget, gpointer user_data);

static GtkWidget *create_timer_list(CtkConfig *);

static guint signals[1];

GType ctk_config_get_type(
    void
)
{
    static GType ctk_config_type = 0;

    if (!ctk_config_type) {
        static const GTypeInfo ctk_config_info = {
            sizeof (CtkConfigClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) ctk_config_class_init, /* class_init */
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (CtkConfig),
            0,    /* n_preallocs */
            NULL, /* instance_init */
        };

        ctk_config_type = g_type_register_static
            (GTK_TYPE_VBOX, "CtkConfig", &ctk_config_info, 0);
    }

    return ctk_config_type;
}

static void ctk_config_class_init(CtkConfigClass *ctk_config_class)
{
    signals[0] = g_signal_new("slider_text_entry_toggled",                   
                              G_OBJECT_CLASS_TYPE(ctk_config_class),
                              G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                              g_cclosure_marshal_VOID__VOID,
                              G_TYPE_NONE, 0);
}

GtkWidget* ctk_config_new(ConfigProperties *conf)
{
    GObject *object;
    CtkConfig *ctk_config;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *frame;
    GtkWidget *image;
    GtkWidget *label;
    GtkWidget *hseparator;
    GtkWidget *check_button;
    gboolean b;

    guint8 *image_buffer = NULL;
    const nv_image_t *img;

    object = g_object_new(CTK_TYPE_CONFIG, NULL);

    ctk_config = CTK_CONFIG(object);

    ctk_config->conf = conf;

    gtk_box_set_spacing(GTK_BOX(ctk_config), 10);
    
    /* initialize the statusbar widget */

    ctk_config->status_bar.widget = gtk_statusbar_new();
    ctk_config->status_bar.prev_message_id = 0;
    
    gtk_statusbar_set_has_resize_grip
        (GTK_STATUSBAR(ctk_config->status_bar.widget), FALSE);
    
    /* XXX force the status bar window to be vertially centered */

    gtk_misc_set_alignment
        (GTK_MISC(GTK_STATUSBAR(ctk_config->status_bar.widget)->label),
         0.0, 0.5);
    
    /* initialize the tooltips widget */

    ctk_config->tooltips.object = gtk_tooltips_new();

    /* banner */

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ctk_config), hbox, FALSE, FALSE, 0);

    frame = gtk_frame_new(NULL);
    gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
    
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);

    img = &configuration_banner_image;

    image_buffer = decompress_image_data(img);

    image = gtk_image_new_from_pixbuf
        (gdk_pixbuf_new_from_data(image_buffer, GDK_COLORSPACE_RGB,
                                  FALSE, 8, img->width, img->height,
                                  img->width * img->bytes_per_pixel,
                                  free_decompressed_image, NULL));

    gtk_container_add(GTK_CONTAINER(frame), image);
    
    /* "nvidia-settings Configuration" */

    hbox = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start(GTK_BOX(ctk_config), hbox, FALSE, FALSE, 0);

    label = gtk_label_new("nvidia-settings Configuration");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    hseparator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(hbox), hseparator, TRUE, TRUE, 0);
    
    /* check buttons: Enable tooltips, Display statusbar, and Display
       slider text entries */

    vbox = gtk_vbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(ctk_config), vbox, FALSE, FALSE, 0);

    /* enable tooltips */
    
    label = gtk_label_new("Enable ToolTips");

    check_button = gtk_check_button_new();
    gtk_container_add(GTK_CONTAINER(check_button), label);
    
    b = !!(ctk_config->conf->booleans & CONFIG_PROPERTIES_TOOLTIPS);

    if (b) {
        gtk_tooltips_enable(ctk_config->tooltips.object);
    } else {
        gtk_tooltips_disable(ctk_config->tooltips.object);
    }

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), b);
    gtk_box_pack_start(GTK_BOX(vbox), check_button, FALSE, FALSE, 0);
   
    g_signal_connect(G_OBJECT(check_button), "toggled",
                     G_CALLBACK(tooltips_toggled), ctk_config);
    
    ctk_config_set_tooltip(ctk_config, check_button, __tooltip_help);

    /* display status bar */

    label = gtk_label_new("Display Status Bar");

    check_button = gtk_check_button_new();
    gtk_container_add(GTK_CONTAINER(check_button), label);
    
    b = !!(ctk_config->conf->booleans & CONFIG_PROPERTIES_DISPLAY_STATUS_BAR);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), b);
    gtk_box_pack_start(GTK_BOX(vbox), check_button, FALSE, FALSE, 0);
    
    g_signal_connect(G_OBJECT(check_button), "toggled",
                     G_CALLBACK(display_status_bar_toggled), ctk_config);

    ctk_config_set_tooltip(ctk_config, check_button, __status_bar_help);
    
    /* display the slider text entries */

    label = gtk_label_new("Slider Text Entries");
    
    check_button = gtk_check_button_new();
    gtk_container_add(GTK_CONTAINER(check_button), label);
    
    b = !!(ctk_config->conf->booleans & CONFIG_PROPERTIES_SLIDER_TEXT_ENTRIES);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), b);
    gtk_box_pack_start(GTK_BOX(vbox), check_button, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(check_button), "toggled",
                     G_CALLBACK(slider_text_entries_toggled), ctk_config);

    ctk_config_set_tooltip(ctk_config, check_button,
                           __slider_text_entries_help);

    /* specify display name in config file */

    label = gtk_label_new("Include X Display Names in the Config File");

    check_button = gtk_check_button_new();
    gtk_container_add(GTK_CONTAINER(check_button), label);

    b = !!(ctk_config->conf->booleans &
           CONFIG_PROPERTIES_INCLUDE_DISPLAY_NAME_IN_CONFIG_FILE);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), b);
    
    gtk_box_pack_start(GTK_BOX(vbox), check_button, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(check_button), "toggled",
                     G_CALLBACK(display_name_toggled), ctk_config);

    ctk_config_set_tooltip(ctk_config, check_button, __x_display_names_help);
    
    /* timer list */
    
    ctk_config->timer_list_box = gtk_hbox_new(FALSE, 0);
    ctk_config->timer_list = create_timer_list(ctk_config);
    ctk_config->timer_list_visible = FALSE;

    gtk_box_pack_start(GTK_BOX(ctk_config), ctk_config->timer_list_box,
                       TRUE, TRUE, 0); 
    
    gtk_widget_show_all(GTK_WIDGET(ctk_config));

    return GTK_WIDGET(ctk_config);
}



void ctk_config_statusbar_message(CtkConfig *ctk_config, const char *fmt, ...)
{
    va_list ap;
    gchar *str;

    if ((!ctk_config) ||
        (!ctk_config->status_bar.widget) ||
        (!(ctk_config->conf->booleans &
           CONFIG_PROPERTIES_DISPLAY_STATUS_BAR))) {
        return;
    }

    if (ctk_config->status_bar.prev_message_id) {
        gtk_statusbar_remove(GTK_STATUSBAR(ctk_config->status_bar.widget),
                             1, ctk_config->status_bar.prev_message_id);
    }

    va_start(ap, fmt);
    str = g_strdup_vprintf(fmt, ap);
    va_end(ap);
    
    ctk_config->status_bar.prev_message_id =
        gtk_statusbar_push
        (GTK_STATUSBAR(ctk_config->status_bar.widget), 1, str);

    g_free(str);
    
} /* ctk_config_statusbar_message() */



GtkWidget* ctk_config_get_statusbar(CtkConfig *ctk_config)
{
    return ctk_config->status_bar.widget;
}



void ctk_config_set_tooltip(CtkConfig *ctk_config,
                            GtkWidget *widget,
                            const gchar *text)

{
    gtk_tooltips_set_tip(ctk_config->tooltips.object, widget, text, NULL);
}


static void display_status_bar_toggled(
    GtkWidget *widget,
    gpointer user_data
)
{
    CtkConfig *ctk_config = CTK_CONFIG(user_data);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        gtk_widget_show(ctk_config->status_bar.widget);
        ctk_config->conf->booleans |= CONFIG_PROPERTIES_DISPLAY_STATUS_BAR;
    } else {
        gtk_widget_hide(ctk_config->status_bar.widget);

        if (ctk_config->status_bar.prev_message_id) {
            gtk_statusbar_remove(GTK_STATUSBAR(ctk_config->status_bar.widget),
                                 1, ctk_config->status_bar.prev_message_id);
        }
    
        ctk_config->status_bar.prev_message_id = 0;

        ctk_config->conf->booleans &= ~CONFIG_PROPERTIES_DISPLAY_STATUS_BAR;
    }
}

static void tooltips_toggled(GtkWidget *widget, gpointer user_data)
{
    CtkConfig *ctk_config = CTK_CONFIG(user_data);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        gtk_tooltips_enable(ctk_config->tooltips.object);
        ctk_config->conf->booleans |= CONFIG_PROPERTIES_TOOLTIPS;
    } else {
        gtk_tooltips_disable(ctk_config->tooltips.object);
        ctk_config->conf->booleans &= ~CONFIG_PROPERTIES_TOOLTIPS;
    }
}


static void slider_text_entries_toggled(GtkWidget *widget, gpointer user_data)
{
    CtkConfig *ctk_config = CTK_CONFIG(user_data);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        ctk_config->conf->booleans |= CONFIG_PROPERTIES_SLIDER_TEXT_ENTRIES;
    } else {
        ctk_config->conf->booleans &= ~CONFIG_PROPERTIES_SLIDER_TEXT_ENTRIES;
    }
    
    g_signal_emit(ctk_config, signals[0], 0);
}

static void display_name_toggled(GtkWidget *widget, gpointer user_data)
{
    CtkConfig *ctk_config = CTK_CONFIG(user_data);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        ctk_config->conf->booleans |=
            CONFIG_PROPERTIES_INCLUDE_DISPLAY_NAME_IN_CONFIG_FILE;
    } else {
        ctk_config->conf->booleans &=
            ~CONFIG_PROPERTIES_INCLUDE_DISPLAY_NAME_IN_CONFIG_FILE;
    }
}



gboolean ctk_config_slider_text_entry_shown(CtkConfig *ctk_config)
{
    return !!(ctk_config->conf->booleans &
              CONFIG_PROPERTIES_SLIDER_TEXT_ENTRIES);
}

GtkTextBuffer *ctk_config_create_help(GtkTextTagTable *table)
{
    GtkTextIter i;
    GtkTextBuffer *b;

    b = gtk_text_buffer_new(table);
    
    gtk_text_buffer_get_iter_at_offset(b, &i, 0);

    ctk_help_title(b, &i, "nvidia-settings Configuration Help");
    
    ctk_help_heading(b, &i, "Enable ToolTips");
    ctk_help_para(b, &i, __tooltip_help);

    ctk_help_heading(b, &i, "Display Status Bar");
    ctk_help_para(b, &i, __status_bar_help);

    ctk_help_heading(b, &i, "Slider Text Entries");
    ctk_help_para(b, &i, __slider_text_entries_help);
    
    ctk_help_heading(b, &i, "Include X Display Names in the Config File");
    ctk_help_para(b, &i, __x_display_names_help);
    
    ctk_help_heading(b, &i, "Active Timers");
    ctk_help_para(b, &i, "Some attributes are polled periodically "
                  "to ensure the reported values are up-to-date. "
                  " Each row in the 'Active Timers' table reflects "
                  "the configuration of one of these timers and "
                  "controls how frequently, if at all, a given "
                  "attribute is polled.  The 'Description' field "
                  "describes the function of a timer, the 'Enabled' "
                  "field allows enabling/disabling it, the 'Time "
                  "Interval' field controls the delay between two "
                  "consecutive polls (in milliseconds).  The Active "
                  "Timers table is only visible when timers are active.");

    ctk_help_finish(b);

    return b;
    
} /* create_help() */

/****************************************************************************/

/* max time interval is 60 seconds, and min time interval is .1 seconds */

#define MAX_TIME_INTERVAL (60 * 1000)
#define MIN_TIME_INTERVAL (100)

static void time_interval_renderer_func(GtkTreeViewColumn*, GtkCellRenderer*,
                                        GtkTreeModel*, GtkTreeIter*, gpointer);

static void time_interval_edited(GtkCellRendererText*,
                                 const gchar*, const gchar*, gpointer);

static void timer_enable_toggled(GtkCellRendererToggle*, gchar*, gpointer);



enum {
    ENABLE_COLUMN = 0,
    TIME_INTERVAL_COLUMN,
    DESCRIPTION_COLUMN,
    FUNCTION_COLUMN,
    DATA_COLUMN,
    HANDLE_COLUMN,
    NUM_COLUMNS,
};

static GtkWidget *create_timer_list(CtkConfig *ctk_config)
{
    GtkTreeModel *model;
    GtkWidget *treeview;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkWidget *sw;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *alignment;
    
    sw = gtk_scrolled_window_new(NULL, NULL);
    
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

    ctk_config->list_store =
        gtk_list_store_new(NUM_COLUMNS,
                           G_TYPE_BOOLEAN,  /* ENABLE_COLUMN */
                           G_TYPE_UINT,     /* TIME_INTERVAL_COLUMN */
                           G_TYPE_STRING,   /* DESCRIPTION_COLUMN */
                           G_TYPE_POINTER,  /* FUNCTION_COLUMN */
                           G_TYPE_POINTER,  /* DATA_COLUMN */
                           G_TYPE_UINT);    /* HANDLE_COLUMN */
    
    model = GTK_TREE_MODEL(ctk_config->list_store);
    
    treeview = gtk_tree_view_new_with_model(model);
    
    g_object_unref(ctk_config->list_store);

    /* Enable */

    renderer = gtk_cell_renderer_toggle_new();
    g_signal_connect(renderer, "toggled",
                     G_CALLBACK(timer_enable_toggled), ctk_config);
    column = gtk_tree_view_column_new_with_attributes("Enabled", renderer,
                                                      "active", ENABLE_COLUMN,
                                                      NULL);
    
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    gtk_tree_view_column_set_resizable(column, FALSE);

    /* Description */
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Description",
                                                      renderer,
                                                      "text",
                                                      DESCRIPTION_COLUMN,
                                                      NULL);
    
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    
    /* Time interval */

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes
        ("Time Interval", renderer, "editable", TRUE, NULL);

    g_signal_connect(renderer, "edited",
                     G_CALLBACK(time_interval_edited), ctk_config);
    
    gtk_tree_view_column_set_cell_data_func(column,
                                            renderer,
                                            time_interval_renderer_func,
                                            GINT_TO_POINTER
                                            (TIME_INTERVAL_COLUMN),
                                            NULL);
    
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    gtk_tree_view_column_set_resizable(column, FALSE);


    gtk_container_add(GTK_CONTAINER(sw), treeview);

    vbox = gtk_vbox_new(FALSE, 5);
    
    label = gtk_label_new("Active Timers:");
    alignment = gtk_alignment_new(0.0, 0.0, 0, 0);
    gtk_container_add(GTK_CONTAINER(alignment), label);
    gtk_box_pack_start(GTK_BOX(vbox), alignment, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);
    
    /* create the tooltip for the treeview (can't do it per column) */
    
    ctk_config_set_tooltip(ctk_config, treeview,
                           "The Active Timers describe operations that "
                           "nvidia-settings will perform at regular "
                           "intervals.");
    
    return vbox;

} /* create_timer_list() */


static void time_interval_renderer_func(GtkTreeViewColumn *tree_column,
                                        GtkCellRenderer   *cell,
                                        GtkTreeModel      *model,
                                        GtkTreeIter       *iter,
                                        gpointer           data)
{
    gint column = GPOINTER_TO_INT(data);
    guint value;
    gchar str[32];
    
    gtk_tree_model_get(model, iter, column, &value, -1);

    snprintf(str, 32, "%d ms", value);
    
    g_object_set(GTK_CELL_RENDERER(cell), "text", str, NULL);
}



static void time_interval_edited(GtkCellRendererText *cell,
                                 const gchar         *path_string,
                                 const gchar         *new_text,
                                 gpointer             user_data)
{
    CtkConfig *ctk_config = CTK_CONFIG(user_data);
    GtkTreeModel *model = GTK_TREE_MODEL(ctk_config->list_store);
    GtkTreePath *path;
    GtkTreeIter iter;
    guint handle;
    GSourceFunc function;
    gpointer data;
    guint interval;

    interval = strtol(new_text, (char **)NULL, 10);
    
    if ((interval == 0) ||
        (interval == LONG_MAX) ||
        (interval == LONG_MIN)) return;

    if (interval > MAX_TIME_INTERVAL) interval = MAX_TIME_INTERVAL;
    if (interval < MIN_TIME_INTERVAL) interval = MIN_TIME_INTERVAL;

    path = gtk_tree_path_new_from_string(path_string);
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_path_free(path);

    gtk_list_store_set(ctk_config->list_store, &iter,
                       TIME_INTERVAL_COLUMN, interval, -1);
    
    gtk_tree_model_get(model, &iter,
                       HANDLE_COLUMN, &handle,
                       FUNCTION_COLUMN, &function,
                       DATA_COLUMN, &data,
                       -1);
    
    g_source_remove(handle);
    
    handle = g_timeout_add(interval, function, data);

    gtk_list_store_set(ctk_config->list_store, &iter,
                       HANDLE_COLUMN, handle, -1);
}
     
static void timer_enable_toggled(GtkCellRendererToggle *cell,
                                 gchar                 *path_string,
                                 gpointer               user_data)
{
    CtkConfig *ctk_config = CTK_CONFIG(user_data);
    GtkTreeModel *model = GTK_TREE_MODEL(ctk_config->list_store);
    GtkTreePath *path;
    GtkTreeIter iter;
    guint handle;
    GSourceFunc function;
    gpointer data;
    guint interval;
    gboolean enabled;
    
    path = gtk_tree_path_new_from_string(path_string);
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_path_free(path);

    gtk_tree_model_get(model, &iter,
                       ENABLE_COLUMN, &enabled,
                       HANDLE_COLUMN, &handle,
                       FUNCTION_COLUMN, &function,
                       DATA_COLUMN, &data,
                       TIME_INTERVAL_COLUMN, &interval,
                       -1);

    enabled ^= 1;

    if (enabled) {
        handle = g_timeout_add(interval, function, data);
        gtk_list_store_set(ctk_config->list_store, &iter,
                           HANDLE_COLUMN, handle, -1);
    } else {
        g_source_remove(handle);
    }

    gtk_list_store_set(ctk_config->list_store, &iter,
                       ENABLE_COLUMN, enabled, -1);
}

void ctk_config_add_timer(CtkConfig *ctk_config,
                          guint interval,
                          gchar *descr,
                          GSourceFunc function,
                          gpointer data)
{
    GtkTreeIter iter;
    guint handle;

    handle = g_timeout_add(interval, function, data);
    
    gtk_list_store_append(ctk_config->list_store, &iter);
    gtk_list_store_set(ctk_config->list_store, &iter,
                       ENABLE_COLUMN, TRUE,
                       TIME_INTERVAL_COLUMN, interval,
                       DESCRIPTION_COLUMN, descr,
                       FUNCTION_COLUMN, function,
                       HANDLE_COLUMN, handle,
                       DATA_COLUMN, data, -1);

    /* make the timer list visible if it is not */

    if (!ctk_config->timer_list_visible) {
        gtk_box_pack_start(GTK_BOX(ctk_config->timer_list_box),
                           ctk_config->timer_list,
                           TRUE, TRUE, 0); 
        gtk_widget_show_all(ctk_config->timer_list_box);
        ctk_config->timer_list_visible = TRUE;
    }
}

void ctk_config_remove_timer(CtkConfig *ctk_config, GSourceFunc function)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GSourceFunc func;
    gboolean valid;
    guint handle;
    
    model = GTK_TREE_MODEL(ctk_config->list_store);

    valid = gtk_tree_model_get_iter_first(model, &iter);
    while (valid) {
        gtk_tree_model_get(model, &iter, FUNCTION_COLUMN, &func,
                           HANDLE_COLUMN, &handle, -1);
        if (func == function) {
            g_source_remove(handle);
            gtk_list_store_remove(ctk_config->list_store, &iter);
            break;
        }
        valid = gtk_tree_model_iter_next(model, &iter);
    }

    /* if there are no more entries, hide the timer list */

    valid = gtk_tree_model_get_iter_first(model, &iter);
    if (!valid) {
        gtk_container_remove(GTK_CONTAINER(ctk_config->timer_list_box),
                             ctk_config->timer_list);
        ctk_config->timer_list_visible = FALSE;
    }
}


