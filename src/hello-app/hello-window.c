#include "hello-window.h"
#include "hello-image-viewer.h"
#include "config.h"

struct _HelloWindow {
    GtkApplicationWindow parent_instance;
    
    GtkWidget *greeting_label;
    GtkWidget *close_button;
    GtkWidget *open_image_button;
    char *greeting_text;
};

G_DEFINE_FINAL_TYPE(HelloWindow, hello_window, GTK_TYPE_APPLICATION_WINDOW)

enum {
    PROP_GREETING = 1,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/* Default values */
#define DEFAULT_WINDOW_WIDTH  400
#define DEFAULT_WINDOW_HEIGHT 300
#define DEFAULT_GREETING_TEXT "Hello World!"
#define DEFAULT_WINDOW_TITLE  "Hello World"

/* Forward declarations */
static void on_file_dialog_response(GObject *source, GAsyncResult *result, gpointer user_data);

static void
hello_window_set_property(GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    HelloWindow *window = HELLO_WINDOW(object);

    switch (prop_id) {
        case PROP_GREETING:
            hello_window_set_greeting(window, g_value_get_string(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void
hello_window_get_property(GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    HelloWindow *window = HELLO_WINDOW(object);

    switch (prop_id) {
        case PROP_GREETING:
            g_value_set_string(value, hello_window_get_greeting(window));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void
on_close_button_clicked(GtkButton *button, HelloWindow *window)
{
    GtkApplication *app;
    
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(HELLO_IS_WINDOW(window));
    
    app = gtk_window_get_application(GTK_WINDOW(window));
    if (app != NULL) {
        g_application_quit(G_APPLICATION(app));
    }
}

static void
on_open_image_button_clicked(GtkButton *button, HelloWindow *window)
{
    GtkFileDialog *dialog;
    GtkFileFilter *filter;
    GListStore *filters;
    GtkApplication *app;
    
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(HELLO_IS_WINDOW(window));
    
    app = gtk_window_get_application(GTK_WINDOW(window));
    
    /* Create file dialog */
    dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Open Image");
    
    /* Create image file filter */
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Image Files");
    gtk_file_filter_add_mime_type(filter, "image/png");
    gtk_file_filter_add_mime_type(filter, "image/jpeg");
    gtk_file_filter_add_mime_type(filter, "image/gif");
    gtk_file_filter_add_mime_type(filter, "image/svg+xml");
    gtk_file_filter_add_mime_type(filter, "image/webp");
    
    /* Add filter to dialog */
    filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
    g_list_store_append(filters, filter);
    gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));
    
    /* Show the dialog asynchronously */
    gtk_file_dialog_open(dialog,
                         GTK_WINDOW(window),
                         NULL,
                         (GAsyncReadyCallback)on_file_dialog_response,
                         g_object_ref(app));
    
    g_object_unref(dialog);
    g_object_unref(filters);
    g_object_unref(filter);
}

static void
on_file_dialog_response(GObject *source, GAsyncResult *result, gpointer user_data)
{
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    GtkApplication *app = GTK_APPLICATION(user_data);
    GFile *file;
    GError *error = NULL;
    
    file = gtk_file_dialog_open_finish(dialog, result, &error);
    
    if (file != NULL) {
        char *filename;
        HelloImageViewer *viewer;
        
        filename = g_file_get_path(file);
        
        /* Create and show image viewer */
        viewer = hello_image_viewer_new(app, filename);
        if (viewer != NULL) {
            gtk_window_present(GTK_WINDOW(viewer));
        }
        
        g_free(filename);
        g_object_unref(file);
    } else if (error != NULL) {
        /* User cancelled or error occurred - just ignore silently */
        g_error_free(error);
    }
    
    g_object_unref(app);
}

static void
hello_window_dispose(GObject *object)
{
    HelloWindow *window = HELLO_WINDOW(object);

    g_clear_object(&window->greeting_label);
    g_clear_object(&window->close_button);
    g_clear_object(&window->open_image_button);

    G_OBJECT_CLASS(hello_window_parent_class)->dispose(object);
}

static void
hello_window_finalize(GObject *object)
{
    HelloWindow *window = HELLO_WINDOW(object);

    g_free(window->greeting_text);

    G_OBJECT_CLASS(hello_window_parent_class)->finalize(object);
}

static void
hello_window_class_init(HelloWindowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    object_class->dispose = hello_window_dispose;
    object_class->finalize = hello_window_finalize;
    object_class->set_property = hello_window_set_property;
    object_class->get_property = hello_window_get_property;

    /* Properties */
    properties[PROP_GREETING] = 
        g_param_spec_string("greeting",
                            "Greeting",
                            "The greeting message to display",
                            DEFAULT_GREETING_TEXT,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties(object_class, N_PROPS, properties);

    /* Set up template */
    gtk_widget_class_set_template_from_resource(widget_class, "/com/example/HelloApp/resources/hello-window.ui");
    gtk_widget_class_bind_template_child(widget_class, HelloWindow, greeting_label);
    gtk_widget_class_bind_template_child(widget_class, HelloWindow, close_button);
    gtk_widget_class_bind_template_child(widget_class, HelloWindow, open_image_button);
    gtk_widget_class_bind_template_callback(widget_class, on_close_button_clicked);
    gtk_widget_class_bind_template_callback(widget_class, on_open_image_button_clicked);
}

static void
hello_window_init(HelloWindow *window)
{
    /* Initialize the template */
    gtk_widget_init_template(GTK_WIDGET(window));
    
    /* Set default properties */
    gtk_window_set_title(GTK_WINDOW(window), DEFAULT_WINDOW_TITLE);
    gtk_window_set_default_size(GTK_WINDOW(window), DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    
    /* Set initial greeting text */
    window->greeting_text = g_strdup(DEFAULT_GREETING_TEXT);
    gtk_label_set_text(GTK_LABEL(window->greeting_label), window->greeting_text);
}

HelloWindow *
hello_window_new(GtkApplication *app)
{
    g_return_val_if_fail(GTK_IS_APPLICATION(app), NULL);

    return g_object_new(HELLO_TYPE_WINDOW,
                        "application", app,
                        NULL);
}

void
hello_window_set_greeting(HelloWindow *window, const char *text)
{
    g_return_if_fail(HELLO_IS_WINDOW(window));
    
    if (g_strcmp0(window->greeting_text, text) == 0)
        return;
    
    g_free(window->greeting_text);
    window->greeting_text = g_strdup(text ? text : DEFAULT_GREETING_TEXT);
    
    if (window->greeting_label)
        gtk_label_set_text(GTK_LABEL(window->greeting_label), window->greeting_text);
    
    g_object_notify_by_pspec(G_OBJECT(window), properties[PROP_GREETING]);
}

const char *
hello_window_get_greeting(HelloWindow *window)
{
    g_return_val_if_fail(HELLO_IS_WINDOW(window), NULL);
    
    return window->greeting_text;
}
