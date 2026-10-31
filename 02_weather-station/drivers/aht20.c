#include <stdint.h>
#include <string.h>

#include "./aht20.h"
#include "./i2c.h"

/*
  AHT20 Product manual; Sensor reading process
  ===

  1. Wait 40ms after power-on.

  Before reading the temperature and humidity values, first check whether the calibration
  enable bit Bit [3] of the status word is 1 (you can get a byte of status word by sending 0x71).

  If not 1, need to send 0xbe command (for initialization), this command parameter has
  two bytes, the first byte is 0x08, the second byte is 0x00, and then **wait for 10ms**.

  2. Send the 0xAC command directly (trigger measurement).
  The parameter of this command has two bytes, the first byte
  is 0x33 and the second byte is 0x00.

  3. Wait for 80ms to wait for the measurement to be completed.
  If the read status word Bit [7] is 0, it indicates that the
  measurement is completed, and then six bytes can be read
  in a row; otherwise, continue to wait.

  4. After receiving six bytes, the next byte is the CRC check
  data, the user can read it as needed
*/

enum aht20_states {
  AHT20_WAIT_FOR_POWER_ON,
  AHT20_CHECK_CALIBRATION,
  AHT20_TRIGGER_INIT,
  AHT20_WAIT_FOR_INIT,
  AHT20_TRIGGER_MEASUREMENT,
  AHT20_WAIT_FOR_MEASUREMENT,
  AHT20_CHECK_MEASUREMENT_STATUS,
  AHT20_RETRY_MEASUREMENT,
  AHT20_READ_MEASUREMENT_DATA,
  AHT20_IDLE,
  AHT20_ERROR
};

struct aht20_context {
  volatile enum aht20_states state;
  uint8_t address;
  uint32_t state_started_ms;
  uint32_t measurement_started_ms;
  bool is_initialized;
  bool is_reading_ready;
  bool has_error;
  float humidity;
  float temperature_c;
};

static const uint8_t aht20_init_commands[3] = {0xBE, 0x08, 0x00};
static const uint8_t aht20_measure_commands[3] = {0xAC, 0x33, 0x00};
static const uint8_t aht20_measurement_timeout = 200U;
static struct aht20_context sensor;

void aht20_init(uint8_t address) {
  i2c_init();

  memset(&sensor, 0, sizeof(sensor));

  sensor.address = address;
  sensor.state = AHT20_IDLE;
}

void aht20_begin(uint32_t now_ms) {
  if (!aht20_is_busy()) {
    sensor.state = sensor.is_initialized ? AHT20_TRIGGER_MEASUREMENT : AHT20_WAIT_FOR_POWER_ON;
    sensor.state_started_ms = now_ms;
    sensor.is_reading_ready = false;
    sensor.has_error = false;
  }
}

void aht20_update(uint32_t now_ms) {
  uint32_t elapsed_ms = (uint32_t)(now_ms - sensor.state_started_ms);

  switch (sensor.state) {
    case AHT20_WAIT_FOR_POWER_ON: {
      if (elapsed_ms > 40) {
        sensor.state = AHT20_CHECK_CALIBRATION;
      }

    } break;

    case AHT20_CHECK_CALIBRATION: {
      // Check whether the calibration enable bit Bit [3] of the status word is 1
      uint8_t buffer[1];

      if (!i2c_read(sensor.address, buffer, sizeof buffer)) {
        sensor.state = AHT20_ERROR;
      } else if ((buffer[0] & (1U << 3)) == 0) {
        sensor.state = AHT20_TRIGGER_INIT;
      } else {
        sensor.is_initialized = true;
        sensor.state = AHT20_TRIGGER_MEASUREMENT;
      }

    } break;

    case AHT20_TRIGGER_INIT: {
      if (i2c_write(sensor.address, aht20_init_commands, 3)) {
        sensor.state = AHT20_WAIT_FOR_INIT;
        sensor.state_started_ms = now_ms;
      } else {
        sensor.state = AHT20_ERROR;
      }

    } break;

    case AHT20_WAIT_FOR_INIT: {
      if (elapsed_ms > 10) {
        sensor.is_initialized = true;
        sensor.state = AHT20_TRIGGER_MEASUREMENT;
      }
    } break;

    case AHT20_TRIGGER_MEASUREMENT: {
      if (i2c_write(sensor.address, aht20_measure_commands, 3)) {
        sensor.state = AHT20_WAIT_FOR_MEASUREMENT;
        sensor.state_started_ms = now_ms;
        sensor.measurement_started_ms = now_ms;
      } else {
        sensor.state = AHT20_ERROR;
      }

    } break;

    case AHT20_WAIT_FOR_MEASUREMENT: {
      if (elapsed_ms > 80) {
        sensor.state = AHT20_CHECK_MEASUREMENT_STATUS;
      }
    } break;

    case AHT20_RETRY_MEASUREMENT: {
      if (elapsed_ms > 10) {
        sensor.state = AHT20_CHECK_MEASUREMENT_STATUS;
      }
    } break;

    case AHT20_CHECK_MEASUREMENT_STATUS: {
      uint8_t buffer[1];

      bool is_timeout =
          (uint32_t)(now_ms - sensor.measurement_started_ms) >= aht20_measurement_timeout;

      if (!is_timeout && i2c_read(sensor.address, buffer, sizeof buffer)) {
        // If bit [7] is 0, it indicates that the measurement is completed
        if ((buffer[0] & (1U << 7)) == 0) {
          sensor.state = AHT20_READ_MEASUREMENT_DATA;
        } else {
          sensor.state = AHT20_RETRY_MEASUREMENT;
          sensor.state_started_ms = now_ms;
        }
      } else {
        sensor.state = AHT20_ERROR;
      }

    } break;

    case AHT20_READ_MEASUREMENT_DATA: {
      uint8_t buffer[6];

      if (i2c_read(sensor.address, buffer, sizeof buffer)) {
        // Humidity data are located in the 1st, 2nd and 3rd bytes (partially) of the reading
        uint32_t h_data = ((buffer[1] << 16) | (buffer[2] << 8) | (buffer[3] & 0b11110000)) >> 4;

        // Temperature data are located in the 3rd (partially), 4th, and 5th bytes of the reading
        uint32_t t_data = ((buffer[3] & 0b00001111) << 16) | (buffer[4] << 8) | buffer[5];

        sensor.humidity = (float)((h_data / 1048576.0f) * 100);
        sensor.temperature_c = (float)(((t_data / 1048576.0f) * 200) - 50);

        sensor.is_reading_ready = true;
        sensor.state = AHT20_IDLE;
      } else {
        sensor.state = AHT20_ERROR;
      }
    } break;

    case AHT20_ERROR: {
      i2c_reset();

      sensor.has_error = true;
      sensor.state = AHT20_IDLE;
      sensor.is_initialized = false;

    } break;

    case AHT20_IDLE: {
      // wait
    } break;
  }
}

bool aht20_is_busy(void) {
  return sensor.state != AHT20_IDLE;
}

// Retrieve readings only ONCE
bool aht20_take_reading(float readings[]) {
  if (!sensor.is_reading_ready) {
    return false;
  }

  readings[0] = sensor.humidity;
  readings[1] = sensor.temperature_c;

  sensor.is_reading_ready = false;
  return true;
}

// Retrieve error state ONCE
bool aht20_take_error(void) {
  if (sensor.has_error) {
    sensor.has_error = false;
    return true;
  }

  return false;
}