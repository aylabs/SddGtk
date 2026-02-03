/* blur-cache.c - LRU cache implementation for blur results
 *
 * Copyright (C) 2026 Image Viewer Contributors
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#include "blur-cache.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Private structures */

typedef struct _BlurCacheEntry {
    gchar *key;                    // "pixbuf_hash:intensity" format
    GdkPixbuf *blurred_pixbuf;     // Cached blur result
    gsize memory_size;             // Memory footprint
    gint64 access_timestamp;       // Last access for LRU
    gint reference_count;          // Thread safety reference counting
    GList *lru_node;               // Node in LRU linked list
} BlurCacheEntry;

struct _BlurCache {
    /* Cache storage */
    GHashTable *cache_table;       // key -> BlurCacheEntry mapping
    GList *lru_list;               // LRU ordered list (most recent first)
    
    /* Limits and statistics */
    guint max_entries;
    gsize max_memory;
    guint current_entries;
    gsize current_memory;
    
    /* Performance statistics */
    guint64 hit_count;
    guint64 miss_count;
    guint64 eviction_count;
    
    /* Thread safety */
    GMutex cache_mutex;
};

/* Private helper functions */

static void cache_entry_free(BlurCacheEntry *entry) {
    if (entry) {
        g_free(entry->key);
        if (entry->blurred_pixbuf) {
            g_object_unref(entry->blurred_pixbuf);
        }
        g_free(entry);
    }
}

static gint64 get_timestamp_microseconds(void) {
    return g_get_monotonic_time();
}

static BlurCacheEntry* cache_entry_create(const gchar *key, 
                                        GdkPixbuf *pixbuf, 
                                        gsize memory_size) {
    BlurCacheEntry *entry = g_malloc0(sizeof(BlurCacheEntry));
    if (!entry) {
        return NULL;
    }
    
    entry->key = g_strdup(key);
    entry->blurred_pixbuf = g_object_ref(pixbuf);
    entry->memory_size = memory_size;
    entry->access_timestamp = get_timestamp_microseconds();
    entry->reference_count = 1;
    entry->lru_node = NULL;
    
    return entry;
}

static void update_lru_order(BlurCache *cache, BlurCacheEntry *entry) {
    // Remove from current position if already in list
    if (entry->lru_node) {
        cache->lru_list = g_list_remove_link(cache->lru_list, entry->lru_node);
        g_list_free(entry->lru_node);
    }
    
    // Add to front of list (most recently used)
    cache->lru_list = g_list_prepend(cache->lru_list, entry);
    entry->lru_node = cache->lru_list;
    entry->access_timestamp = get_timestamp_microseconds();
}

static gboolean should_evict_for_memory(BlurCache *cache, gsize new_entry_size) {
    return (cache->current_memory + new_entry_size > cache->max_memory);
}

static gboolean should_evict_for_count(BlurCache *cache) {
    return (cache->current_entries >= cache->max_entries);
}

static void evict_lru_entry(BlurCache *cache) {
    if (!cache->lru_list) {
        return;
    }
    
    // Find least recently used entry (last in list)
    GList *last = g_list_last(cache->lru_list);
    BlurCacheEntry *lru_entry = (BlurCacheEntry*)last->data;
    
    // Store memory size before removal (entry will be freed by hash table)
    gsize entry_memory_size = lru_entry->memory_size;
    
    // Remove from LRU list first
    cache->lru_list = g_list_remove_link(cache->lru_list, last);
    g_list_free(last);
    
    // Remove from hash table (this will automatically free the entry via GDestroyNotify)
    g_hash_table_remove(cache->cache_table, lru_entry->key);
    
    // Update statistics
    cache->current_entries--;
    cache->current_memory -= entry_memory_size;
    cache->eviction_count++;
}

/* Public API Implementation */

BlurCache* blur_cache_create(guint max_entries, gsize max_memory_bytes) {
    if (max_entries == 0 || max_memory_bytes < 1024 * 1024) { // Minimum 1MB
        return NULL;
    }
    
    BlurCache *cache = g_malloc0(sizeof(BlurCache));
    if (!cache) {
        return NULL;
    }
    
    cache->cache_table = g_hash_table_new_full(g_str_hash, g_str_equal,
                                              NULL, (GDestroyNotify)cache_entry_free);
    if (!cache->cache_table) {
        g_free(cache);
        return NULL;
    }
    
    cache->max_entries = max_entries;
    cache->max_memory = max_memory_bytes;
    cache->current_entries = 0;
    cache->current_memory = 0;
    cache->lru_list = NULL;
    
    cache->hit_count = 0;
    cache->miss_count = 0;
    cache->eviction_count = 0;
    
    g_mutex_init(&cache->cache_mutex);
    
    return cache;
}

GdkPixbuf* blur_cache_get(BlurCache *cache, const gchar *pixbuf_hash, gdouble intensity) {
    if (!cache || !pixbuf_hash) {
        return NULL;
    }
    
    gchar *key = blur_cache_make_key(pixbuf_hash, intensity);
    if (!key) {
        return NULL;
    }
    
    g_mutex_lock(&cache->cache_mutex);
    
    BlurCacheEntry *entry = g_hash_table_lookup(cache->cache_table, key);
    GdkPixbuf *result = NULL;
    
    if (entry) {
        // Cache hit - update LRU order and return pixbuf
        update_lru_order(cache, entry);
        result = g_object_ref(entry->blurred_pixbuf);
        cache->hit_count++;
    } else {
        // Cache miss
        cache->miss_count++;
    }
    
    g_mutex_unlock(&cache->cache_mutex);
    g_free(key);
    
    return result;
}

gboolean blur_cache_put(BlurCache *cache,
                       const gchar *pixbuf_hash,
                       gdouble intensity,
                       GdkPixbuf *blurred_pixbuf) {
    if (!cache || !pixbuf_hash || !blurred_pixbuf) {
        return FALSE;
    }
    
    gchar *key = blur_cache_make_key(pixbuf_hash, intensity);
    if (!key) {
        return FALSE;
    }
    
    gsize memory_size = blur_cache_calculate_pixbuf_size(blurred_pixbuf);
    
    g_mutex_lock(&cache->cache_mutex);
    
    // Check if entry already exists
    if (g_hash_table_contains(cache->cache_table, key)) {
        g_mutex_unlock(&cache->cache_mutex);
        g_free(key);
        return TRUE; // Already cached
    }
    
    // Perform eviction if necessary
    while (should_evict_for_count(cache) || should_evict_for_memory(cache, memory_size)) {
        if (!cache->lru_list) {
            // Cache is empty but limits exceeded - entry too large
            g_mutex_unlock(&cache->cache_mutex);
            g_free(key);
            return FALSE;
        }
        evict_lru_entry(cache);
    }
    
    // Create and insert new entry
    BlurCacheEntry *entry = cache_entry_create(key, blurred_pixbuf, memory_size);
    if (!entry) {
        g_mutex_unlock(&cache->cache_mutex);
        g_free(key);
        return FALSE;
    }
    
    g_hash_table_insert(cache->cache_table, entry->key, entry);
    update_lru_order(cache, entry);
    
    cache->current_entries++;
    cache->current_memory += memory_size;
    
    g_mutex_unlock(&cache->cache_mutex);
    g_free(key);
    
    return TRUE;
}

void blur_cache_remove(BlurCache *cache, const gchar *pixbuf_hash) {
    if (!cache || !pixbuf_hash) {
        return;
    }
    
    g_mutex_lock(&cache->cache_mutex);
    
    // Find and remove all entries matching the pixbuf hash
    GHashTableIter iter;
    gpointer key, value;
    GList *keys_to_remove = NULL;
    
    g_hash_table_iter_init(&iter, cache->cache_table);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        const gchar *entry_key = (const gchar*)key;
        if (g_str_has_prefix(entry_key, pixbuf_hash)) {
            keys_to_remove = g_list_prepend(keys_to_remove, g_strdup(entry_key));
        }
    }
    
    // Remove found entries
    for (GList *node = keys_to_remove; node; node = node->next) {
        BlurCacheEntry *entry = g_hash_table_lookup(cache->cache_table, node->data);
        if (entry) {
            if (entry->lru_node) {
                cache->lru_list = g_list_remove_link(cache->lru_list, entry->lru_node);
                g_list_free(entry->lru_node);
            }
            cache->current_entries--;
            cache->current_memory -= entry->memory_size;
            g_hash_table_remove(cache->cache_table, node->data);
        }
    }
    
    g_list_free_full(keys_to_remove, g_free);
    g_mutex_unlock(&cache->cache_mutex);
}

void blur_cache_clear(BlurCache *cache) {
    if (!cache) {
        return;
    }
    
    g_mutex_lock(&cache->cache_mutex);
    
    g_hash_table_remove_all(cache->cache_table);
    g_list_free(cache->lru_list);
    cache->lru_list = NULL;
    
    cache->current_entries = 0;
    cache->current_memory = 0;
    
    g_mutex_unlock(&cache->cache_mutex);
}

void blur_cache_destroy(BlurCache *cache) {
    if (!cache) {
        return;
    }
    
    blur_cache_clear(cache);
    g_hash_table_unref(cache->cache_table);
    g_mutex_clear(&cache->cache_mutex);
    g_free(cache);
}

void blur_cache_get_stats(BlurCache *cache, BlurCacheStats *stats) {
    if (!cache || !stats) {
        return;
    }
    
    g_mutex_lock(&cache->cache_mutex);
    
    stats->current_entries = cache->current_entries;
    stats->max_entries = cache->max_entries;
    stats->current_memory = cache->current_memory;
    stats->max_memory = cache->max_memory;
    stats->hit_count = cache->hit_count;
    stats->miss_count = cache->miss_count;
    stats->eviction_count = cache->eviction_count;
    
    g_mutex_unlock(&cache->cache_mutex);
}

gsize blur_cache_get_memory_usage(BlurCache *cache) {
    if (!cache) {
        return 0;
    }
    
    g_mutex_lock(&cache->cache_mutex);
    gsize usage = cache->current_memory;
    g_mutex_unlock(&cache->cache_mutex);
    
    return usage;
}

gboolean blur_cache_is_memory_pressure(BlurCache *cache, gdouble threshold) {
    if (!cache || threshold < 0.0 || threshold > 1.0) {
        return FALSE;
    }
    
    g_mutex_lock(&cache->cache_mutex);
    gboolean pressure = (cache->current_memory >= threshold * cache->max_memory);
    g_mutex_unlock(&cache->cache_mutex);
    
    return pressure;
}

guint blur_cache_evict_lru(BlurCache *cache, guint min_entries_to_free) {
    if (!cache) {
        return 0;
    }
    
    g_mutex_lock(&cache->cache_mutex);
    
    guint evicted = 0;
    while (evicted < min_entries_to_free && cache->lru_list) {
        evict_lru_entry(cache);
        evicted++;
    }
    
    g_mutex_unlock(&cache->cache_mutex);
    
    return evicted;
}

/* Utility Functions */

gchar* blur_cache_make_key(const gchar *pixbuf_hash, gdouble intensity) {
    if (!pixbuf_hash) {
        return NULL;
    }
    
    gdouble rounded_intensity = blur_cache_round_intensity(intensity);
    return g_strdup_printf("%s:%.1f", pixbuf_hash, rounded_intensity);
}

gsize blur_cache_calculate_pixbuf_size(GdkPixbuf *pixbuf) {
    if (!pixbuf) {
        return 0;
    }
    
    gint width = gdk_pixbuf_get_width(pixbuf);
    gint height = gdk_pixbuf_get_height(pixbuf);
    gint channels = gdk_pixbuf_get_n_channels(pixbuf);
    
    // Calculate pixel data size + estimated overhead
    gsize pixel_data_size = width * height * channels;
    gsize overhead = 1024; // Approximate GdkPixbuf overhead
    
    return pixel_data_size + overhead;
}


gdouble blur_cache_round_intensity(gdouble intensity) {
    // Round to 0.1 precision for consistent cache keys
    return round(intensity * 10.0) / 10.0;
}
