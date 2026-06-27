#pragma once

#include "esphome/components/json/json_util.h"

namespace esphome {
namespace rtl433 {

class Rtl433Listener {
 public:
  virtual void on_message(JsonObject root) = 0;
  virtual ~Rtl433Listener() = default;
};

}  // namespace rtl433
}  // namespace esphome
