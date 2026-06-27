#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../rtl433_listener.h"

namespace esphome {
namespace rtl433 {

class Rtl433BinarySensor : public binary_sensor::BinarySensor, public Rtl433Listener {
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
    // Truthy: non-zero number or non-empty/non-null value = ON
    JsonVariant val = root[field_];
    bool state = false;
    if (val.is<bool>()) {
      state = val.as<bool>();
    } else if (val.is<int>()) {
      state = val.as<int>() != 0;
    } else if (val.is<float>()) {
      state = val.as<float>() != 0.0f;
    } else if (val.is<const char *>()) {
      const char *s = val.as<const char *>();
      state = (s != nullptr && s[0] != '\0');
    } else {
      state = !val.isNull();
    }
    publish_state(state);
  }

 private:
  std::string device_model_;
  std::string device_id_;
  std::string field_;
};

}  // namespace rtl433
}  // namespace esphome
