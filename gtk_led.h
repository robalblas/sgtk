#ifndef __GTK_LED_H__
#define __GTK_LED_H__


#include <gdk/gdk.h>
#include <gtk/gtkvbox.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* 
 * --- Macros for conversion and type checking 
 */
#define GTK_LED(obj) \
   GTK_CHECK_CAST (obj, gtk_led_get_type (), GtkLed)
#define GTK_LED_CLASS(klass) \
   GTK_CHECK_CLASS_CAST (klass, gtk_led_get_type, GtkLedClass)
#define GTK_IS_LED(obj) \
   GTK_CHECK_TYPE (obj, gtk_led_get_type ())

/*
 * --- Defining data structures.
 */

typedef struct _GtkLed      GtkLed;
typedef struct _GtkLedClass GtkLedClass;

/*
 * Here's the led data
 */
struct _GtkLed
{
  GtkWidget vbox;

  gint *values;
  gint num_values;
};


/*
 * Here's the class data.
 */
struct _GtkLedClass
{
  GtkWidgetClass parent_class;
};


/*
 * Function prototypes
 */
GtkWidget* gtk_led_new (void);
GtkWidget* gtk_led_new_with_label (const char *);
void gtk_led_set_value (GtkLed *led, int index, int value);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_LED_H__ */

