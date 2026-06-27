#pragma once

#include "esphome/components/text_sensor/text_sensor.h"
#include "../rtl433_listener.h"

namespace esphome {
namespace rtl433 {

class Rtl433TextSensor : public text_sensor::TextSensor, public Rtl433Listener {
 public:
  void set_device_model(const std::string &model) { device_model_ = model; }
  void set_device_id(const std::string &id) { device_id_ = id; }
  void set_field(const std::string &field) { field_ = field; }

  void on_message(JsonObject root) override {
    if (!root.containsKey("model"))
      return;
    if (root["model"].as<std::string>() != device_model_)
      return;
    if (!device_id_.empty()) {
      if (!root.containsKey("id"))
        return;
      if (root["id"].as<std::string>() != device_id_)
        return;
    }
    if (!root.containsKey(field_))
      return;
    publish_state(root[field_].as<std::string>());
  }

 private:
  std::string device_model_;
  std::string device_id_;
  std::string field_;
};

}  // namespace rtl433
}  // namespace esphome
