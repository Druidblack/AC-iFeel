#pragma once

#include <cstdint>
#include <string>

#include "esphome/components/remote_transmitter/remote_transmitter.h"
#include "esphome/core/component.h"

#include "kelon168_protocol.h"

#ifdef USE_MQTT
#include "esphome/components/mqtt/mqtt_client.h"
#endif

namespace esphome {
namespace kelon168_mqtt_ir {

class Kelon168MqttIr : public Component {
 public:
  void set_transmitter(remote_transmitter::RemoteTransmitterComponent *transmitter) { this->transmitter_ = transmitter; }
  void set_topic(const std::string &topic) { this->topic_ = topic; }
  void set_qos(uint8_t qos) { this->qos_ = qos; }
  void set_send_times(uint8_t send_times) { this->send_times_ = send_times; }

  void setup() override;
  void dump_config() override;

 protected:
  void handle_payload_(const std::string &payload);
  bool parse_hex_payload_(const std::string &payload, Kelon168Data *data);
  bool parse_hex_byte_(const std::string &token, uint8_t *out);

  remote_transmitter::RemoteTransmitterComponent *transmitter_{nullptr};
  std::string topic_;
  uint8_t qos_{0};
  uint8_t send_times_{1};
};

}  // namespace kelon168_mqtt_ir
}  // namespace esphome
