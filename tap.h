// exocortex-wasm-runtime: TAP protocol implementation
// Zero heap allocation, fixed-size buffers, WASM-ready

#ifndef EXOCORTEX_TAP_H
#define EXOCORTEX_TAP_H

#include <string.h>
#include <stdio.h>

// ── Configuration ─────────────────────────────────────────────────────

#define TAP_MAX_SENSORS       16
#define TAP_MEMORY_SLOTS      32
#define TAP_KEY_SIZE          32
#define TAP_VALUE_SIZE        64
#define TAP_HISTORY_DEPTH     8
#define TAP_RESULT_SIZE       128

// ── Sensor Store ──────────────────────────────────────────────────────

typedef struct {
    int  values[TAP_HISTORY_DEPTH];
    int  count;
    int  write_pos;
} SensorHistory;

static SensorHistory g_sensors[TAP_MAX_SENSORS];
static int g_sensors_initialized = 0;

static void ensure_sensors_init(void) {
    if (!g_sensors_initialized) {
        memset(g_sensors, 0, sizeof(g_sensors));
        g_sensors_initialized = 1;
    }
}

// ── Memory Store ──────────────────────────────────────────────────────

typedef struct {
    char key[TAP_KEY_SIZE];
    char value[TAP_VALUE_SIZE];
    int  used;
} MemorySlot;

static MemorySlot g_memory[TAP_MEMORY_SLOTS];
static int g_memory_initialized = 0;

static void ensure_memory_init(void) {
    if (!g_memory_initialized) {
        memset(g_memory, 0, sizeof(g_memory));
        g_memory_initialized = 1;
    }
}

// ── Result Buffer ─────────────────────────────────────────────────────

static char g_result_buf[TAP_RESULT_SIZE];

// ── Public API ────────────────────────────────────────────────────────

// Feed a sensor reading (for testing / simulation)
static void tap_feed_sensor(int sensor_id, int value) {
    ensure_sensors_init();
    if (sensor_id < 0 || sensor_id >= TAP_MAX_SENSORS) return;
    SensorHistory* h = &g_sensors[sensor_id];
    h->values[h->write_pos] = value;
    h->write_pos = (h->write_pos + 1) % TAP_HISTORY_DEPTH;
    if (h->count < TAP_HISTORY_DEPTH) h->count++;
}

// Read the latest sensor value
int tap_sense(int sensor_id) {
    ensure_sensors_init();
    if (sensor_id < 0 || sensor_id >= TAP_MAX_SENSORS) return -1;
    SensorHistory* h = &g_sensors[sensor_id];
    if (h->count == 0) return 0;
    int idx = (h->write_pos - 1 + TAP_HISTORY_DEPTH) % TAP_HISTORY_DEPTH;
    return h->values[idx];
}

// Store a key-value pair in memory
static void tap_store(const char* key, const char* value) {
    ensure_memory_init();
    if (!key || !value) return;
    // Find existing or empty slot
    int empty = -1;
    for (int i = 0; i < TAP_MEMORY_SLOTS; i++) {
        if (g_memory[i].used && strncmp(g_memory[i].key, key, TAP_KEY_SIZE) == 0) {
            strncpy(g_memory[i].value, value, TAP_VALUE_SIZE - 1);
            g_memory[i].value[TAP_VALUE_SIZE - 1] = '\0';
            return;
        }
        if (!g_memory[i].used && empty < 0) empty = i;
    }
    if (empty >= 0) {
        g_memory[empty].used = 1;
        strncpy(g_memory[empty].key, key, TAP_KEY_SIZE - 1);
        g_memory[empty].key[TAP_KEY_SIZE - 1] = '\0';
        strncpy(g_memory[empty].value, value, TAP_VALUE_SIZE - 1);
        g_memory[empty].value[TAP_VALUE_SIZE - 1] = '\0';
    }
}

// Recall a value by key
const char* tap_recall(const char* query) {
    ensure_memory_init();
    if (!query) return "(null)";
    for (int i = 0; i < TAP_MEMORY_SLOTS; i++) {
        if (g_memory[i].used && strncmp(g_memory[i].key, query, TAP_KEY_SIZE) == 0) {
            return g_memory[i].value;
        }
    }
    return "(not found)";
}

// Predict the next sensor value using simple linear extrapolation
float tap_predict(int sensor_id) {
    ensure_sensors_init();
    if (sensor_id < 0 || sensor_id >= TAP_MAX_SENSORS) return 0.0f;
    SensorHistory* h = &g_sensors[sensor_id];
    if (h->count < 2) return (float)tap_sense(sensor_id);

    // Use last two values for linear extrapolation
    int idx1 = (h->write_pos - 1 + TAP_HISTORY_DEPTH) % TAP_HISTORY_DEPTH;
    int idx2 = (h->write_pos - 2 + TAP_HISTORY_DEPTH) % TAP_HISTORY_DEPTH;
    float v1 = (float)h->values[idx1];
    float v2 = (float)h->values[idx2];
    return v1 + (v1 - v2); // linear extrapolation
}

// Reset all state (useful for tests)
void tap_reset(void) {
    g_sensors_initialized = 0;
    g_memory_initialized = 0;
    memset(g_sensors, 0, sizeof(g_sensors));
    memset(g_memory, 0, sizeof(g_memory));
    memset(g_result_buf, 0, sizeof(g_result_buf));
}

#endif // EXOCORTEX_TAP_H
