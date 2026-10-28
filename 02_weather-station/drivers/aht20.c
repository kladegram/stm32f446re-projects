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
  AHT20_TRIGGER_INIT,
  AHT20_WAIT_FOR_INIT,
  AHT20_TRIGGER_MEASUMERENT,
  AHT20_WAIT_FOR_MEASUMERENT,
  AHT20_CHECK_MEASUMERENT_STATUS,
  AHT20_READ_MEASUMERENT_DATA,
  AHT20_IDLE
};

struct aht20_context {
  volatile enum aht20_states state;
  uint8_t address;
  uint32_t state_started_ms;
  bool is_initialized;
  bool is_reading_ready;
  float humidity;
  float temperature_c;
};

uint8_t t_init_commands[3] = {0xBE, 0x08, 0x00};
uint8_t t_measure_commands[3] = {0xAC, 0x33, 0x00};

static struct aht20_context sensor;

void aht20_init(uint8_t address) {
  i2c_init();

  memset(&sensor, 0, sizeof(sensor));

  sensor.address = address;
  sensor.state = AHT20_IDLE;
}

void aht20_begin(uint32_t now_ms) {
  sensor.state = sensor.is_initialized ? AHT20_TRIGGER_MEASUMERENT : AHT20_WAIT_FOR_POWER_ON;
  sensor.state_started_ms = now_ms;
}

void aht20_update(uint32_t now_ms) {
  uint32_t elapsed = (uint32_t)(now_ms - sensor.state_started_ms);

  switch (sensor.state) {
    case AHT20_WAIT_FOR_POWER_ON: {
      if (elapsed > 40) {
        sensor.state = AHT20_TRIGGER_INIT;
      }

    } break;

    case AHT20_TRIGGER_INIT: {
      i2c_write(sensor.address, t_init_commands, 3);

      sensor.state = AHT20_WAIT_FOR_INIT;
      sensor.state_started_ms = now_ms;

    } break;

    case AHT20_WAIT_FOR_INIT: {
      if (elapsed > 10) {
        // TODO: Read status and check bit 3
        sensor.is_initialized = true;

        sensor.state = AHT20_TRIGGER_MEASUMERENT;
      }
    } break;

    case AHT20_TRIGGER_MEASUMERENT: {
      i2c_write(sensor.address, t_measure_commands, 3);

      sensor.state = AHT20_WAIT_FOR_MEASUMERENT;
      sensor.state_started_ms = now_ms;

    } break;

    case AHT20_WAIT_FOR_MEASUMERENT: {
      if (elapsed > 80) {
        sensor.state = AHT20_CHECK_MEASUMERENT_STATUS;
      }
    } break;

    case AHT20_CHECK_MEASUMERENT_STATUS: {
      uint8_t buffer[1];

      i2c_read(sensor.address, buffer, sizeof buffer);

      // If bit [7] is 0, it indicates that the measurement is completed
      if ((buffer[0] & (1U << 7)) == 0) {
        sensor.state = AHT20_READ_MEASUMERENT_DATA;
      }

    } break;

    case AHT20_READ_MEASUMERENT_DATA: {
      uint8_t buffer[6];

      i2c_read(sensor.address, buffer, sizeof buffer);

      // Humidity data are located in the 1st, 2nd and 3rd bytes (partially) of the reading
      uint32_t h_data = ((buffer[1] << 16) | (buffer[2] << 8) | (buffer[3] & 0b11110000)) >> 4;

      // Temperature data are located in the 3rd (partially), 4th, and 5th bytes of the reading
      uint32_t t_data = ((buffer[3] & 0b00001111) << 16) | (buffer[4] << 8) | buffer[5];

      sensor.humidity = (float)((h_data / 1048576.0f) * 100);
      sensor.temperature_c = (float)(((t_data / 1048576.0f) * 200) - 50);
      sensor.is_reading_ready = true;
      sensor.state = AHT20_IDLE;
    } break;

    case AHT20_IDLE: {
      // wait
    } break;
  }
}

bool aht20_is_busy(void) {
  return sensor.state != AHT20_IDLE;
}

// Allow to read only ONCE
bool aht20_take_reading(float readings[]) {
  if (!sensor.is_reading_ready) {
    return false;
  }

  readings[0] = sensor.humidity;
  readings[1] = sensor.temperature_c;

  sensor.is_reading_ready = false;
  return true;
}