/* blur-cache.h - LRU cache for blur processing results
 *
 * Copyright (C) 2026 Image Viewer Contributors
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#pragma once

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

typedef struct _BlurCache BlurCache;

/**
 * BlurCacheStats:
 * @current_entries: Number of cached blur results
 * @max_entries: Maximum cache entries allowed
 * @current_memory: Current memory usage in bytes
 * @max_memory: Maximum memory limit in bytes
 * @hit_count: Number of cache hits
 * @miss_count: Number of cache misses
 * @eviction_count: Number of LRU evictions performed
 *
 * Cache performance and usage statistics
 */
typedef struct {
    guint current_entries;
    guint max_entries;
    gsize current_memory;
    gsize max_memory;
    guint64 hit_count;
    guint64 miss_count;
    guint64 eviction_count;
} BlurCacheStats;

/* Core Cache Functions */

/**
 * blur_cache_create:
 * @max_entries: Maximum number of cached blur variants
 * @max_memory_bytes: Maximum total memory usage in bytes
 *
 * Creates LRU cache for blur results with specified limits.
 * Empty cache ready for use with memory tracking initialized.
 *
 * Typical usage: max_entries=5, max_memory_bytes=150MB
 *
 * Returns: New BlurCache instance, or NULL on failure
 */
BlurCache* blur_cache_create(guint max_entries, gsize max_memory_bytes);

/**
 * blur_cache_get:
 * @cache: BlurCache instance
 * @pixbuf_hash: Hash of original pixbuf
 * @intensity: Blur intensity to look up (rounded to 0.1 precision)
 *
 * Retrieves cached blur result if available. Cache hit updates LRU order.
 * Returned pixbuf has incremented reference count.
 *
 * Performance: O(1) average case lookup, O(1) amortized LRU update
 *
 * Returns: Cached pixbuf with added reference, or NULL if not found
 */
GdkPixbuf* blur_cache_get(BlurCache *cache, 
                         const gchar *pixbuf_hash, 
                         gdouble intensity);

/**
 * blur_cache_put:
 * @cache: BlurCache instance
 * @pixbuf_hash: Hash of original pixbuf (must be unique)
 * @intensity: Blur intensity (rounded to 0.1 precision)
 * @blurred_pixbuf: Blur result to cache
 *
 * Stores blur result in cache with LRU eviction if necessary.
 * Takes ownership of blurred_pixbuf reference, may evict older entries
 * to stay within memory limits.
 *
 * Returns: TRUE if successfully cached, FALSE if rejected
 */
gboolean blur_cache_put(BlurCache *cache,
                       const gchar *pixbuf_hash,
                       gdouble intensity,
                       GdkPixbuf *blurred_pixbuf);

/**
 * blur_cache_remove:
 * @cache: BlurCache instance
 * @pixbuf_hash: Hash of original pixbuf
 *
 * Removes all cached blur variants for the given image.
 * Used when image is unloaded or changed.
 */
void blur_cache_remove(BlurCache *cache, const gchar *pixbuf_hash);

/**
 * blur_cache_clear:
 * @cache: BlurCache instance
 *
 * Removes all entries from cache and frees associated memory.
 * Used for memory pressure relief or cache reset.
 */
void blur_cache_clear(BlurCache *cache);

/**
 * blur_cache_destroy:
 * @cache: BlurCache instance to destroy
 *
 * Destroys cache and frees all resources including cached pixbufs.
 */
void blur_cache_destroy(BlurCache *cache);

/* Cache Management and Statistics */

/**
 * blur_cache_get_stats:
 * @cache: BlurCache instance
 * @stats: Output structure for statistics
 *
 * Retrieves current cache performance and usage statistics.
 * Useful for monitoring cache effectiveness and memory usage.
 */
void blur_cache_get_stats(BlurCache *cache, BlurCacheStats *stats);

/**
 * blur_cache_get_memory_usage:
 * @cache: BlurCache instance
 *
 * Returns current memory usage in bytes for cache monitoring.
 *
 * Returns: Current memory usage in bytes
 */
gsize blur_cache_get_memory_usage(BlurCache *cache);

/**
 * blur_cache_is_memory_pressure:
 * @cache: BlurCache instance
 * @threshold: Memory pressure threshold (0.0-1.0, e.g., 0.9 for 90%)
 *
 * Checks if cache is approaching memory limits and should trigger
 * proactive eviction or cache clearing.
 *
 * Returns: TRUE if memory usage >= threshold * max_memory
 */
gboolean blur_cache_is_memory_pressure(BlurCache *cache, gdouble threshold);

/**
 * blur_cache_evict_lru:
 * @cache: BlurCache instance
 * @min_entries_to_free: Minimum number of entries to evict
 *
 * Manually triggers LRU eviction to free cache entries.
 * Used for proactive memory management.
 *
 * Returns: Number of entries actually evicted
 */
guint blur_cache_evict_lru(BlurCache *cache, guint min_entries_to_free);

/* Utility Functions */

/**
 * blur_cache_make_key:
 * @pixbuf_hash: Hash string of original pixbuf
 * @intensity: Blur intensity (will be rounded to 0.1 precision)
 *
 * Creates cache key string for given image hash and blur intensity.
 * Key format: "pixbuf_hash:intensity" (e.g., "abc123:2.5")
 *
 * Returns: Newly allocated key string, caller must free with g_free()
 */
gchar* blur_cache_make_key(const gchar *pixbuf_hash, gdouble intensity);

/**
 * blur_cache_calculate_pixbuf_size:
 * @pixbuf: Pixbuf to calculate memory footprint for
 *
 * Calculates memory footprint of pixbuf for cache accounting.
 * Includes pixel data and overhead estimation.
 *
 * Returns: Memory size in bytes
 */
gsize blur_cache_calculate_pixbuf_size(GdkPixbuf *pixbuf);

/**
 * blur_cache_round_intensity:
 * @intensity: Raw intensity value
 *
 * Rounds intensity to cache precision (0.1) for consistent key generation.
 * Ensures cache hits for similar intensity values.
 *
 * Returns: Rounded intensity to 0.1 precision
 */
gdouble blur_cache_round_intensity(gdouble intensity);

G_END_DECLS
