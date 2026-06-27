#pragma once

#include "esphome/components/text_sensor/text_sensor.h"
#include "../rtl433_listener.h"

namespace esphome {
namespace rtl433 {

class Rtl433TextSensor : public text_sensor::TextSensor, public Rtl433Listener {
 public:
  void set_sensor_device_model(const std::string &model) { sensor_device_model_ = model; }
  void set_sensor_device_id(const std::string &id) { sensor_device_id_ = id; }
  void set_sensor_field(const std::string &field) { sensor_field_ = field; }

  void on_message(JsonObject root) override {
    if (!root.containsKey("model"))
      return;
    if (root["model"].as<std::string>() != sensor_device_model_)
      return;
    if (!sensor_device_id_.empty()) {
      if (!root.containsKey("id"))
        return;
      if (root["id"].as<std::string>() != sensor_device_id_)
        return;
    }
    if (!root.containsKey(sensor_field_))
      return;
    publish_state(root[sensor_field_].as<std::string>());
  }

 private:
  std::string sensor_device_model_;
  std::string sensor_device_id_;
  std::string sensor_field_;
};

}  // namespace rtl433
}  // namespace esphome
