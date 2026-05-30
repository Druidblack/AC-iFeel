#include "kelon168_mqtt_ir.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <vector>

#include "esphome/core/log.h"

namespace esphome {
namespace kelon168_mqtt_ir {

static const char *const TAG = "kelon168_mqtt_ir";

void Kelon168MqttIr::setup() {
#ifdef USE_MQTT
  if (this->transmitter_ == nullptr) {
    ESP_LOGE(TAG, "remote_transmitter is not configured");
    this->mark_failed();
    return;
  }
  if (this->topic_.empty()) {
    ESP_LOGE(TAG, "MQTT topic is empty");
    this->mark_failed();
    return;
  }
  if (mqtt::global_mqtt_client == nullptr) {
    ESP_LOGE(TAG, "MQTT component is not configured");
    this->mark_failed();
    return;
  }

  mqtt::global_mqtt_client->subscribe(
      this->topic_, [this](const std::string &topic, const std::string &payload) { this->handle_payload_(payload); },
      this->qos_);
  ESP_LOGCONFIG(TAG, "Subscribed to MQTT topic '%s'", this->topic_.c_str());
#else
  ESP_LOGE(TAG, "MQTT support is not enabled");
  this->mark_failed();
#endif
}

void Kelon168MqttIr::dump_config() {
  ESP_LOGCONFIG(TAG, "Kelon168 MQTT IR transmitter:");
  ESP_LOGCONFIG(TAG, "  Topic: %s", this->topic_.c_str());
  ESP_LOGCONFIG(TAG, "  QoS: %u", this->qos_);
  ESP_LOGCONFIG(TAG, "  Send times: %u", this->send_times_);
}

void Kelon168MqttIr::handle_payload_(const std::string &payload) {
  Kelon168Data data;
  if (!this->parse_hex_payload_(payload, &data)) {
    ESP_LOGW(TAG, "Ignoring invalid Kelon168 MQTT payload: '%s'", payload.c_str());
    return;
  }

  if (!Kelon168Protocol::valid_checksum(data)) {
    ESP_LOGW(TAG, "Ignoring Kelon168 MQTT payload with invalid checksum");
    return;
  }

  char buffer[KELON168_STATE_LENGTH * 3 + 1];
  size_t pos = 0;
  for (uint8_t i = 0; i < KELON168_STATE_LENGTH; i++) {
    pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%02X%s", data.state[i],
                    i + 1 == KELON168_STATE_LENGTH ? "" : " ");
  }
  ESP_LOGI(TAG, "Sending Kelon168 IR from MQTT: %s (command=0x%02X)", buffer, data.command());

  auto call = this->transmitter_->transmit();
  Kelon168Protocol().encode(call.get_data(), data);
  call.set_send_times(this->send_times_);
  call.perform();
}

bool Kelon168MqttIr::parse_hex_payload_(const std::string &payload, Kelon168Data *data) {
  std::string normalized = payload;
  for (char &ch : normalized) {
    if (ch == ',' || ch == ';' || ch == ':' || ch == '[' || ch == ']' || ch == '\n' || ch == '\r' || ch == '\t')
      ch = ' ';
  }

  std::vector<std::string> tokens;
  std::istringstream iss(normalized);
  std::string token;
  while (iss >> token) {
    tokens.push_back(token);
  }

  std::vector<uint8_t> bytes;
  if (tokens.size() == 1) {
    std::string compact;
    for (char ch : tokens[0]) {
      if (std::isxdigit(static_cast<unsigned char>(ch)))
        compact.push_back(ch);
    }
    if (compact.size() != KELON168_STATE_LENGTH * 2)
      return false;
    for (size_t i = 0; i < compact.size(); i += 2) {
      uint8_t value;
      if (!this->parse_hex_byte_(compact.substr(i, 2), &value))
        return false;
      bytes.push_back(value);
    }
  } else {
    for (std::string t : tokens) {
      if (t.size() >= 2 && t[0] == '0' && (t[1] == 'x' || t[1] == 'X'))
        t = t.substr(2);
      if (t.empty())
        continue;
      uint8_t value;
      if (!this->parse_hex_byte_(t, &value))
        return false;
      bytes.push_back(value);
    }
  }

  if (bytes.size() != KELON168_STATE_LENGTH)
    return false;

  for (uint8_t i = 0; i < KELON168_STATE_LENGTH; i++)
    data->state[i] = bytes[i];
  return true;
}

bool Kelon168MqttIr::parse_hex_byte_(const std::string &token, uint8_t *out) {
  if (token.size() != 2)
    return false;
  if (!std::isxdigit(static_cast<unsigned char>(token[0])) || !std::isxdigit(static_cast<unsigned char>(token[1])))
    return false;

  char *end = nullptr;
  long value = std::strtol(token.c_str(), &end, 16);
  if (end == token.c_str() || *end != '\0' || value < 0 || value > 255)
    return false;
  *out = static_cast<uint8_t>(value);
  return true;
}

}  // namespace kelon168_mqtt_ir
}  // namespace esphome
