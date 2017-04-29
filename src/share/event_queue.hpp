#pragma once

#include "boost_defs.hpp"

#include "types.hpp"
#include <boost/optional.hpp>

namespace krbn {
class event_queue final {
public:
  class queued_event final {
  public:
    class event {
    public:
      enum class type {
        key_code,
        pointing_button,
        pointing_x,
        pointing_y,
        pointing_vertical_wheel,
        pointing_horizontal_wheel,
      };

      event(key_code key_code) : type_(type::key_code),
                                 key_code_(key_code) {
      }

      event(pointing_button pointing_button) : type_(type::pointing_button),
                                               pointing_button_(pointing_button) {
      }

      event(type type,
            int64_t integer_value) : type_(type),
                                     integer_value_(integer_value) {
      }

      type get_type(void) const {
        return type_;
      }

      boost::optional<key_code> get_key_code(void) const {
        if (type_ == type::key_code) {
          return key_code_;
        }
        return boost::none;
      }

      boost::optional<pointing_button> get_pointing_button(void) const {
        if (type_ == type::pointing_button) {
          return pointing_button_;
        }
        return boost::none;
      }

      boost::optional<int64_t> get_integer_value(void) const {
        if (type_ == type::pointing_x ||
            type_ == type::pointing_y ||
            type_ == type::pointing_vertical_wheel ||
            type_ == type::pointing_horizontal_wheel) {
          return integer_value_;
        }
        return boost::none;
      }

      bool operator==(const event& other) const {
        return get_type() == other.get_type() &&
               get_key_code() == other.get_key_code() &&
               get_pointing_button() == other.get_pointing_button() &&
               get_integer_value() == other.get_integer_value();
      }

    private:
      type type_;

      union {
        key_code key_code_;               // For type::key_code
        pointing_button pointing_button_; // For type::pointing_button
        int64_t integer_value_;           // For type::pointing_x, type::pointing_y, type::pointing_vertical_wheel, type::pointing_horizontal_wheel
      };
    };

    queued_event(device_id device_id,
                 uint64_t time_stamp,
                 const class event& event,
                 event_type event_type,
                 const class event& original_event) : device_id_(device_id),
                                                      time_stamp_(time_stamp),
                                                      manipulated_(false),
                                                      valid_(true),
                                                      lazy_(false),
                                                      event_(event),
                                                      event_type_(event_type),
                                                      original_event_(original_event) {
    }

    device_id get_device_id(void) const {
      return device_id_;
    }

    uint64_t get_time_stamp(void) const {
      return time_stamp_;
    }

    bool get_manipulated(void) const {
      return manipulated_;
    }
    void set_manipulated(bool value) {
      manipulated_ = value;
    }

    bool get_valid(void) const {
      return valid_;
    }
    void set_valid(bool value) {
      valid_ = value;
    }

    bool get_lazy(void) const {
      return lazy_;
    }
    void set_lazy(bool value) {
      lazy_ = value;
    }

    const event& get_event(void) const {
      return event_;
    }

    event_type get_event_type(void) const {
      return event_type_;
    }

    const event& get_original_event(void) const {
      return original_event_;
    }

    bool operator==(const queued_event& other) const {
      return get_device_id() == other.get_device_id() &&
             get_time_stamp() == other.get_time_stamp() &&
             get_valid() == other.get_valid() &&
             get_lazy() == other.get_lazy() &&
             get_event() == other.get_event() &&
             get_original_event() == other.get_original_event();
    }

  private:
    device_id device_id_;
    uint64_t time_stamp_;
    bool manipulated_;
    bool valid_;
    bool lazy_;
    event event_;
    event_type event_type_;
    event original_event_;
  };

  event_queue(const event_queue&) = delete;

  event_queue(void) {
  }

  // from physical device
  void emplace_back_event(device_id device_id,
                          uint64_t time_stamp,
                          hid_usage_page usage_page,
                          hid_usage usage,
                          int64_t integer_value) {
    if (auto key_code = types::get_key_code(usage_page, usage)) {
      queued_event::event event(*key_code);
      emplace_back_event(device_id,
                         time_stamp,
                         event,
                         integer_value ? event_type::key_down : event_type::key_up,
                         event);

    } else if (auto pointing_button = types::get_pointing_button(usage_page, usage)) {
      queued_event::event event(*pointing_button);
      emplace_back_event(device_id,
                         time_stamp,
                         event,
                         integer_value ? event_type::key_down : event_type::key_up,
                         event);

    } else {
      switch (usage_page) {
        case hid_usage_page::generic_desktop:
          switch (usage) {
            case hid_usage::gd_x: {
              queued_event::event event(queued_event::event::type::pointing_x, integer_value);
              emplace_back_event(device_id,
                                 time_stamp,
                                 event,
                                 event_type::key_down,
                                 event);
              break;
            }

            case hid_usage::gd_y: {
              queued_event::event event(queued_event::event::type::pointing_y, integer_value);
              emplace_back_event(device_id,
                                 time_stamp,
                                 event,
                                 event_type::key_down,
                                 event);
              break;
            }

            case hid_usage::gd_wheel: {
              queued_event::event event(queued_event::event::type::pointing_vertical_wheel, integer_value);
              emplace_back_event(device_id,
                                 time_stamp,
                                 event,
                                 event_type::key_down,
                                 event);
              break;
            }

            default:
              break;
          }
          break;

        case hid_usage_page::consumer:
          switch (usage) {
            case hid_usage::csmr_acpan: {
              queued_event::event event(queued_event::event::type::pointing_horizontal_wheel, integer_value);
              emplace_back_event(device_id,
                                 time_stamp,
                                 event,
                                 event_type::key_down,
                                 event);
              break;
            }

            default:
              break;
          }
          break;

        default:
          break;
      }
    }
  }

  void emplace_back_event(device_id device_id,
                          uint64_t time_stamp,
                          const queued_event::event& event,
                          event_type event_type,
                          const queued_event::event& original_event) {
    events_.emplace_back(device_id,
                         time_stamp,
                         event,
                         event_type,
                         original_event);
    sort_events();
  }

  void clear_events(void) {
    events_.clear();
  }

  void erase_all_invalid_events(void) {
    events_.erase(std::remove_if(std::begin(events_),
                                 std::end(events_),
                                 [&](const queued_event& e) {
                                   return !e.get_valid();
                                 }),
                  std::end(events_));
  }

  std::vector<queued_event>& get_events(void) {
    return events_;
  }

  static bool compare(const queued_event& v1, const queued_event& v2) {
    // Some devices are send modifier flag and key at the same HID report.
    // For example, a key sends control+up-arrow by this reports.
    //
    //   modifiers: 0x0
    //   keys: 0x0 0x0 0x0 0x0 0x0 0x0
    //
    //   modifiers: 0x1
    //   keys: 0x52 0x0 0x0 0x0 0x0 0x0
    //
    // In this case, macOS does not guarantee the value event order to be modifier first.
    // At least macOS 10.12 or prior sends the up-arrow event first.
    //
    //   ----------------------------------------
    //   Example of hid value events in a single queue at control+up-arrow
    //
    //   1. up-arrow keydown
    //     usage_page:0x7
    //     usage:0x4f
    //     integer_value:1
    //
    //   2. control keydown
    //     usage_page:0x7
    //     usage:0xe1
    //     integer_value:1
    //
    //   3. up-arrow keyup
    //     usage_page:0x7
    //     usage:0x4f
    //     integer_value:0
    //
    //   4. control keyup
    //     usage_page:0x7
    //     usage:0xe1
    //     integer_value:0
    //   ----------------------------------------
    //
    // These events will not be interpreted as intended in this order.
    // Thus, we have to reorder the events.

    if (v1.get_time_stamp() == v2.get_time_stamp()) {
      auto modifier_flag1 = modifier_flag::zero;
      auto modifier_flag2 = modifier_flag::zero;

      if (auto key_code1 = v1.get_event().get_key_code()) {
        modifier_flag1 = types::get_modifier_flag(*key_code1);
      }
      if (auto key_code2 = v2.get_event().get_key_code()) {
        modifier_flag2 = types::get_modifier_flag(*key_code2);
      }

      // If either modifier_flag1 or modifier_flag2 is modifier, reorder it before.

      if (modifier_flag1 == modifier_flag::zero &&
          modifier_flag2 != modifier_flag::zero) {
        // v2 is modifier_flag
        if (v2.get_event_type() == event_type::key_up) {
          return true;
        } else {
          // reorder to v2,v1 if v2 is pressed.
          return false;
        }
      }

      if (modifier_flag1 != modifier_flag::zero &&
          modifier_flag2 == modifier_flag::zero) {
        // v1 is modifier_flag
        if (v1.get_event_type() == event_type::key_up) {
          // reorder to v2,v1 if v1 is released.
          return false;
        } else {
          return true;
        }
      }
    }

    // keep order
    return v1.get_time_stamp() < v2.get_time_stamp();
  }

private:
  void sort_events(void) {
    std::stable_sort(std::begin(events_), std::end(events_), event_queue::compare);
  }

  std::vector<queued_event> events_;
};
} // namespace krbn
