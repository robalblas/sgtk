#include "gtk_led.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
/*
#include "gtkled.h"
*/

static GtkWidgetClass *parent_class = NULL;

/* function decl. */
static void gtk_led_class_init (GtkLedClass *class);
static void gtk_led_init (GtkLed *led);
static void gtk_led_realize (GtkWidget *widget);
static void gtk_led_draw (GtkWidget *widget, GdkRectangle *area);
static void gtk_led_size_request (GtkWidget *widget,
                                    GtkRequisition *req);
static gint gtk_led_expose (GtkWidget *widget, GdkEventExpose *event);
static void gtk_led_destroy (GtkObject *object);

/*
 * gtk_led_get_type
 *
 * Internal class.  Used to defined the GtkLed class to GTK
 */
guint gtk_led_get_type (void)
{
  static guint led_type = 0;

    /* --- If not created yet --- */
    if (!led_type) {

        /* --- Create a led_info object --- */
        GtkTypeInfo led_info =
            {
	      "GtkLed",
	      sizeof (GtkLed),
	      sizeof (GtkLedClass),
	      (GtkClassInitFunc) gtk_led_class_init,
	      (GtkObjectInitFunc) gtk_led_init,
#ifndef __GTK_20__
	      (GtkArgSetFunc) NULL,
              (GtkArgGetFunc) NULL,
#endif
            };
  
        /* --- Tell GTK about it - get a unique identifying key --- */
        led_type = gtk_type_unique (gtk_widget_get_type (), &led_info);
    }
    return led_type;
}


/*
 * gtk_led_class_init
 *
 * Override any methods for the led class that are needed for
 * the led class to behave properly.  Here, the functions that
 * cause painting to occur are overridden.
 *
 * class - object definition class.
 */
static void gtk_led_class_init (GtkLedClass *class)
{
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;

    /* --- Get the widget class --- */
    object_class = (GtkObjectClass *) class;
    widget_class = (GtkWidgetClass *) class;
    parent_class = gtk_type_class (gtk_widget_get_type ());

    /* --- Override object destroy --- */
    object_class->destroy = gtk_led_destroy;

    /* --- Override these methods --- */
    widget_class->realize = gtk_led_realize;
#ifndef __GTK_20__
    widget_class->draw = gtk_led_draw;
#endif
    widget_class->size_request = gtk_led_size_request;
    widget_class->expose_event = gtk_led_expose;
}

/*
 * gtk_led_init
 * 
 * Called each time a led item gets created.
 * This initializes fields in our structure.
 */
static void gtk_led_init (GtkLed *led)
{
    GtkWidget *widget;

    widget = (GtkWidget *) led;
   
    /* --- Initial values --- */
    led->values = NULL;
    led->num_values = 0;
}


/*
 * gtk_led_new
 * 
 * Create a new GtkLed item
 */
GtkWidget *gtk_led_new(void)
{
  GtkWidget *iled,*w,*ledo;
  iled=gtk_type_new(gtk_led_get_type());
  w=gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(w),iled, FALSE, FALSE, 0);
  ledo=w;
  w=gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(w),ledo, FALSE, FALSE, 0);
  ledo=w;
  gtk_object_set_user_data((GtkObject *)ledo,iled);
  return ledo;
}


/*
 * gtk_led_realize
 *
 * Associate the widget with an x-window.
 *
 */
static void gtk_led_realize (GtkWidget *widget)
{
  GtkLed *darea;
  GdkWindowAttr attributes;
  gint attributes_mask;

  /* --- Check for failures --- */
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_LED (widget));

  darea = GTK_LED (widget);
  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

  /* --- attributes to create the window --- */
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;

  /* --- We're passing in x, y, visual and colormap values --- */
  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

  /* --- Create the window --- */
  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), 
                                   &attributes, attributes_mask);
  gdk_window_set_user_data (widget->window, darea);

  widget->style = gtk_style_attach (widget->style, widget->window);
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
}

/*
 * gtk_led_size
 *
 * Custom method to set the size of the led.
 */
/*
void gtk_led_size (GtkLed *led, int size)
{
    g_return_if_fail (led != NULL);
    g_return_if_fail (GTK_IS_LED (led));

    led->num_values = size;
    led->values = g_realloc (led->values, sizeof (gint) * size);
}
*/
/*
 * gtk_led_set_value
 *
 * Custon method to set the siz
 */
void gtk_led_set_value (GtkLed *led, int index, int value)
{
    g_return_if_fail (led != NULL);
    g_return_if_fail (GTK_IS_LED (led));
    g_return_if_fail (index < led->num_values && index >= 0);

    led->values[index] = value;
}


/*
 * gtk_led_draw
 *
 * Draw the widget.
 */
static void gtk_led_draw(GtkWidget *widget, GdkRectangle *area)
{
  GtkLed *led;
  int width;
  int height;

  /* --- Check for obvious problems --- */
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_LED (widget));

  /* --- Make sure it's a drawable widget --- */
  if (GTK_WIDGET_DRAWABLE (widget))
  {
    led = GTK_LED (widget);
 
    /* --- Get height and width --- */
    width = widget->allocation.width - 1;
    height = widget->allocation.height - 1;


    /* --- Display led --- */
/*
    gdk_draw_rectangle (widget->window, 
                        widget->style->fg_gc[GTK_STATE_NORMAL],
                        TRUE,
                        0,0,
                        width,height);
*/
    gdk_draw_arc (widget->window, 
                  widget->style->fg_gc[GTK_STATE_NORMAL],
                  TRUE,
                  0,0,
                  width,height,
                  0,23040);
  }
}


/*
 * gtk_led_size_request
 *
 * How big should the widget be?
 * It can be modified. 
 */
static void gtk_led_size_request (GtkWidget *widget,
                                    GtkRequisition *req)
{
  req->width = 10;
  req->height = 10;
}

/*
 * gtk_led_expose
 *
 * The led widget has been exposed and needs to be painted.
 *
 */
static gint gtk_led_expose(GtkWidget *widget, GdkEventExpose *event)
{
    GtkLed *led;

    /* --- Do error checking --- */
    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_LED (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

    if (event->count > 0) {
        return (FALSE);
    }

    /* --- Get the led widget --- */
    led = GTK_LED (widget);

    /* --- Clear the window --- */
    gdk_window_clear_area (widget->window, 0, 0, 
                           widget->allocation.width,
                           widget->allocation.height);

    /* --- Draw the led --- */
    gtk_led_draw (widget, NULL);
    return (TRUE);
}

static void gtk_led_destroy (GtkObject *object)
{

    GtkLed *led;

    /* --- Check type --- */
    g_return_if_fail (object != NULL);
    g_return_if_fail (GTK_IS_LED (object));

    /* --- Convert to led object --- */
    led = GTK_LED (object);

    /* --- Free memory --- */
    g_free (led->values);

    /* --- Call parent destroy --- */
    GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

/*-------------------------------------------------------------*/

GtkWidget *gtk_led_new_with_label(const gchar *label)
{
  GtkWidget *led_widget;
  GtkWidget *label_widget;
  GtkWidget *widgeto;
  
  led_widget = gtk_led_new();
  label_widget = gtk_label_new(label);
  gtk_misc_set_alignment(GTK_MISC (label_widget), 0.0, 0.5);
/*  
  gtk_container_add(GTK_CONTAINER(led_widget), label_widget);
  gtk_widget_show(label_widget);
*/

  widgeto = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(widgeto), led_widget, FALSE, FALSE, 1);

  gtk_box_pack_start(GTK_BOX(widgeto), label_widget, TRUE, TRUE, 1);
  return led_widget;
}

