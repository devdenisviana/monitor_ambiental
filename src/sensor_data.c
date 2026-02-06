#include "sensor_data.h"
#include "pico/stdlib.h"

#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"
#endif

// Dados globais dos sensores (acesso interno)
static sensor_data_t g_sensor_data;

#ifdef USE_FREERTOS
static SemaphoreHandle_t g_sensor_mutex = NULL;

static void sensor_lock(void) {
    if (g_sensor_mutex) {
        xSemaphoreTake(g_sensor_mutex, portMAX_DELAY);
    }
}

static void sensor_unlock(void) {
    if (g_sensor_mutex) {
        xSemaphoreGive(g_sensor_mutex);
    }
}

void sensor_data_set_mutex(SemaphoreHandle_t mutex) {
    g_sensor_mutex = mutex;
}
#else
static void sensor_lock(void) { }
static void sensor_unlock(void) { }
#endif

void sensor_data_init(void) {
    sensor_lock();
    g_sensor_data.luminosity_lux = 0.0f;
    g_sensor_data.luminosity_valid = false;
    
    g_sensor_data.temperature_c = 0.0f;
    g_sensor_data.humidity_percent = 0.0f;
    g_sensor_data.temp_humidity_valid = false;
    
    g_sensor_data.led_matrix_enabled = true;
    g_sensor_data.led_intensity = LED_INTENSITY_OFF;
    
    g_sensor_data.last_update_ms = 0;
    sensor_unlock();
}

void sensor_data_update(const sensor_data_t *data) {
    if (data == NULL) return;
    
    sensor_lock();
    g_sensor_data.luminosity_lux = data->luminosity_lux;
    g_sensor_data.luminosity_valid = data->luminosity_valid;
    g_sensor_data.temperature_c = data->temperature_c;
    g_sensor_data.humidity_percent = data->humidity_percent;
    g_sensor_data.temp_humidity_valid = data->temp_humidity_valid;
    g_sensor_data.led_matrix_enabled = data->led_matrix_enabled;
    g_sensor_data.led_intensity = data->led_intensity;
    g_sensor_data.last_update_ms = to_ms_since_boot(get_absolute_time());
    sensor_unlock();
}

sensor_data_t sensor_data_get(void) {
    sensor_data_t copy;
    sensor_lock();
    copy = g_sensor_data;
    sensor_unlock();
    return copy;
}

void sensor_data_set_luminosity(float lux, bool valid) {
    sensor_lock();
    g_sensor_data.luminosity_lux = lux;
    g_sensor_data.luminosity_valid = valid;
    g_sensor_data.last_update_ms = to_ms_since_boot(get_absolute_time());
    sensor_unlock();
}

void sensor_data_set_temp_humidity(float temp, float humidity, bool valid) {
    sensor_lock();
    g_sensor_data.temperature_c = temp;
    g_sensor_data.humidity_percent = humidity;
    g_sensor_data.temp_humidity_valid = valid;
    g_sensor_data.last_update_ms = to_ms_since_boot(get_absolute_time());
    sensor_unlock();
}

void sensor_data_set_led_state(bool enabled, led_intensity_t intensity) {
    sensor_lock();
    g_sensor_data.led_matrix_enabled = enabled;
    g_sensor_data.led_intensity = intensity;
    g_sensor_data.last_update_ms = to_ms_since_boot(get_absolute_time());
    sensor_unlock();
}
