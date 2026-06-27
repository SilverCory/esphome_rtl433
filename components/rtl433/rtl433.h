#pragma once

// Include ESPHome headers first, then undef Arduino macros that conflict
// with rtl_433_ESP's internal use of the same names.
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/log.h"
#include "esphome/components/json/json_util.h"
#include "rtl433_listener.h"

#undef yield
#undef millis
#undef micros
#undef delay
#undef delayMicroseconds

#include <rtl_433_ESP.h>

namespace esphome {
namespace rtl433 {

static const char *const TAG = "rtl433";

class Rtl433Component;

class Rtl433MessageTrigger : public Trigger<std::string> {
 public:
  explicit Rtl433MessageTrigger(Rtl433Component *parent);
};

class Rtl433Component : public Component {
 public:
  Rtl433Component() { instance_ = this; }

  void setup() override {
    rf_.initReceiver(RF_MODULE_RECEIVER_GPIO, RF_MODULE_FREQUENCY);
    rf_.setCallback(Rtl433Component::callback_, buffer_, sizeof(buffer_));

    // Apply runtime-settable options after the receiver is initialised.
    rf_.setRSSIThreshold(rssi_threshold_delta_);

    rf_.enableReceiver();
    ESP_LOGI(TAG, "CC1101 receiver started on GPIO %d @ %.2f MHz",
             RF_MODULE_RECEIVER_GPIO, (double) RF_MODULE_FREQUENCY);
  }

  void loop() override { rf_.loop(); }

  // Initialise early so the radio hardware is ready before other components
  // (e.g. sensors) that might depend on decoded messages.
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  void add_on_message_trigger(Rtl433MessageTrigger *trigger) {
    triggers_.push_back(trigger);
  }

  void register_listener(Rtl433Listener *listener) {
    listeners_.push_back(listener);
  }

  // --- Runtime-settable option setters (called by generated code) ---
  void set_rssi_threshold_delta(int v) { rssi_threshold_delta_ = v; }

 private:
  rtl_433_ESP rf_;
  char buffer_[512];
  std::vector<Rtl433MessageTrigger *> triggers_;
  std::vector<Rtl433Listener *> listeners_;

  // Runtime-settable config: RSSI threshold delta (default matches upstream)
  int rssi_threshold_delta_{9};

  static Rtl433Component *instance_;

  static void callback_(char *msg) {
    if (instance_ != nullptr)
      instance_->dispatch_(msg);
  }

  void dispatch_(char *msg) {
    ESP_LOGD(TAG, "Received: %s", msg);
    std::string s(msg);
    for (auto *t : triggers_)
      t->trigger(s);
    // Parse JSON once and notify all registered listeners
    if (!listeners_.empty()) {
      json::parse_json(s, [this](JsonObject root) -> bool {
        for (auto *l : listeners_)
          l->on_message(root);
        return true;
      });
    }
  }
};

// inline: safe in a header-only component (avoids ODR issues if the header
// were ever included from more than one translation unit).
inline Rtl433Component *Rtl433Component::instance_ = nullptr;

// Trigger constructor — registers itself with the parent component.
inline Rtl433MessageTrigger::Rtl433MessageTrigger(Rtl433Component *parent) {
  parent->add_on_message_trigger(this);
}

}  // namespace rtl433
}  // namespace esphome
