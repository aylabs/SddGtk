#include <check.h>
#include <gtk/gtk.h>
#include "src/lib/image-processing.h"

/* Test fixtures and helper functions */

/**
 * create_test_pixbuf:
 * @width: Width in pixels
 * @height: Height in pixels  
 * @has_alpha: Whether to include alpha channel
 * 
 * Creates a test pixbuf with a color pattern for testing.
 * 
 * Returns: (transfer full): New test GdkPixbuf
 */
static GdkPixbuf*
create_test_pixbuf(gint width, gint height, gboolean has_alpha)
{
    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
                                      has_alpha, 8, width, height);
    
    /* Fill with a color pattern for testing */
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    gint rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    gint n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    
    for (gint y = 0; y < height; y++) {
        guchar *row = pixels + y * rowstride;
        for (gint x = 0; x < width; x++) {
            guchar *pixel = row + x * n_channels;
            
            /* Create a test pattern with known RGB values */
            pixel[0] = (x * 255) / width;     /* Red gradient */
            pixel[1] = (y * 255) / height;    /* Green gradient */
            pixel[2] = 128;                   /* Constant blue */
            
            if (has_alpha) {
                pixel[3] = 255; /* Full opacity */
            }
        }
    }
    
    return pixbuf;
}

/* Test cases for image_processor_validate_pixbuf */

START_TEST(test_validate_pixbuf_null_input)
{
    ck_assert(!image_processor_validate_pixbuf(NULL));
}
END_TEST

START_TEST(test_validate_pixbuf_valid_input)
{
    GdkPixbuf *pixbuf = create_test_pixbuf(100, 100, FALSE);
    ck_assert(image_processor_validate_pixbuf(pixbuf));
    g_object_unref(pixbuf);
}
END_TEST

START_TEST(test_validate_pixbuf_with_alpha)
{
    GdkPixbuf *pixbuf = create_test_pixbuf(50, 50, TRUE);
    ck_assert(image_processor_validate_pixbuf(pixbuf));
    g_object_unref(pixbuf);
}
END_TEST

/* Test cases for image_processor_estimate_memory_usage */

START_TEST(test_estimate_memory_usage_small)
{
    gsize usage = image_processor_estimate_memory_usage(100, 100);
    ck_assert_uint_gt(usage, 0);
    ck_assert_uint_lt(usage, 1024 * 1024); /* Should be less than 1MB */
}
END_TEST

START_TEST(test_estimate_memory_usage_large)
{
    gsize usage = image_processor_estimate_memory_usage(1920, 1080);
    ck_assert_uint_gt(usage, 1024 * 1024); /* Should be more than 1MB */
}
END_TEST

/* Test cases for image_processor_convert_to_grayscale */

START_TEST(test_convert_to_grayscale_null_input)
{
    GError *error = NULL;
    GdkPixbuf *result = image_processor_convert_to_grayscale(NULL, &error);
    
    ck_assert_ptr_null(result);
    ck_assert_ptr_nonnull(error);
    ck_assert_int_eq(error->code, IMAGE_PROCESSOR_ERROR_INVALID_INPUT);
    
    g_error_free(error);
}
END_TEST

START_TEST(test_convert_to_grayscale_valid_input)
{
    GdkPixbuf *original = create_test_pixbuf(50, 50, FALSE);
    GError *error = NULL;
    
    GdkPixbuf *grayscale = image_processor_convert_to_grayscale(original, &error);
    
    ck_assert_ptr_nonnull(grayscale);
    ck_assert_ptr_null(error);
    
    /* Verify dimensions are preserved */
    ck_assert_int_eq(gdk_pixbuf_get_width(grayscale), 50);
    ck_assert_int_eq(gdk_pixbuf_get_height(grayscale), 50);
    ck_assert(!gdk_pixbuf_get_has_alpha(grayscale));
    
    g_object_unref(original);
    g_object_unref(grayscale);
}
END_TEST

START_TEST(test_convert_to_grayscale_with_alpha)
{
    GdkPixbuf *original = create_test_pixbuf(30, 30, TRUE);
    GError *error = NULL;
    
    GdkPixbuf *grayscale = image_processor_convert_to_grayscale(original, &error);
    
    ck_assert_ptr_nonnull(grayscale);
    ck_assert_ptr_null(error);
    
    /* Verify alpha channel is preserved */
    ck_assert(gdk_pixbuf_get_has_alpha(grayscale));
    ck_assert_int_eq(gdk_pixbuf_get_n_channels(grayscale), 4);
    
    g_object_unref(original);
    g_object_unref(grayscale);
}
END_TEST

START_TEST(test_convert_to_grayscale_luminance_formula)
{
    /* Create a 1x1 pixbuf with known RGB values to test the formula */
    GdkPixbuf *original = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 1, 1);
    guchar *pixels = gdk_pixbuf_get_pixels(original);
    
    /* Set known RGB values: R=255, G=128, B=64 */
    pixels[0] = 255; /* Red */
    pixels[1] = 128; /* Green */
    pixels[2] = 64;  /* Blue */
    
    GError *error = NULL;
    GdkPixbuf *grayscale = image_processor_convert_to_grayscale(original, &error);
    
    ck_assert_ptr_nonnull(grayscale);
    ck_assert_ptr_null(error);
    
    /* Calculate expected luminance: Y = 0.299*255 + 0.587*128 + 0.114*64 */
    gdouble expected = 0.299 * 255 + 0.587 * 128 + 0.114 * 64;
    guchar expected_gray = (guchar)CLAMP(expected, 0, 255);
    
    guchar *gray_pixels = gdk_pixbuf_get_pixels(grayscale);
    
    /* All RGB channels should have the same grayscale value */
    ck_assert_int_eq(gray_pixels[0], expected_gray);
    ck_assert_int_eq(gray_pixels[1], expected_gray);
    ck_assert_int_eq(gray_pixels[2], expected_gray);
    
    g_object_unref(original);
    g_object_unref(grayscale);
}
END_TEST

/* Test suite setup */

static Suite*
image_processing_suite(void)
{
    Suite *s;
    TCase *tc_validate, *tc_memory, *tc_convert;
    
    s = suite_create("ImageProcessing");
    
    /* Validation test cases */
    tc_validate = tcase_create("Validation");
    tcase_add_test(tc_validate, test_validate_pixbuf_null_input);
    tcase_add_test(tc_validate, test_validate_pixbuf_valid_input);
    tcase_add_test(tc_validate, test_validate_pixbuf_with_alpha);
    suite_add_tcase(s, tc_validate);
    
    /* Memory estimation test cases */
    tc_memory = tcase_create("MemoryEstimation");
    tcase_add_test(tc_memory, test_estimate_memory_usage_small);
    tcase_add_test(tc_memory, test_estimate_memory_usage_large);
    suite_add_tcase(s, tc_memory);
    
    /* Conversion test cases */
    tc_convert = tcase_create("Conversion");
    tcase_add_test(tc_convert, test_convert_to_grayscale_null_input);
    tcase_add_test(tc_convert, test_convert_to_grayscale_valid_input);
    tcase_add_test(tc_convert, test_convert_to_grayscale_with_alpha);
    tcase_add_test(tc_convert, test_convert_to_grayscale_luminance_formula);
    suite_add_tcase(s, tc_convert);
    
    return s;
}

int
main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;
    
    /* Initialize GTK for GdkPixbuf functions */
    gtk_init();
    
    s = image_processing_suite();
    sr = srunner_create(s);
    
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}