#define CATCH_CONFIG_RUNNER
#include "../../vendor/catch/catch.hpp"

#include "pressed_physical_keys_counter.hpp"
#include "thread_utility.hpp"

namespace {
krbn::event_queue::queued_event::event event_a(*(krbn::types::get_key_code("a")));
krbn::event_queue::queued_event::event event_escape(*(krbn::types::get_key_code("escape")));
krbn::event_queue::queued_event::event event_left_shift(*(krbn::types::get_key_code("left_shift")));
krbn::event_queue::queued_event::event event_right_shift(*(krbn::types::get_key_code("right_shift")));
krbn::event_queue::queued_event::event event_spacebar(*(krbn::types::get_key_code("spacebar")));
krbn::event_queue::queued_event::event event_tab(*(krbn::types::get_key_code("tab")));

krbn::event_queue::queued_event::event event_button1(krbn::pointing_button::button1);
krbn::event_queue::queued_event::event event_button2(krbn::pointing_button::button2);
krbn::event_queue::queued_event::event event_button3(krbn::pointing_button::button3);
krbn::event_queue::queued_event::event event_button4(krbn::pointing_button::button4);
} // namespace

TEST_CASE("pressed_physical_keys_counter") {
  krbn::pressed_physical_keys_counter pressed_physical_keys_counter;

  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == true);
  REQUIRE(pressed_physical_keys_counter.is_pointing_button_pressed(krbn::device_id(1)) == false);

  // ----------------------------------------

  pressed_physical_keys_counter.emplace_back_event(krbn::device_id(1), event_spacebar);
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == false);
  REQUIRE(pressed_physical_keys_counter.is_pointing_button_pressed(krbn::device_id(1)) == false);
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(2)) == true);

  pressed_physical_keys_counter.erase_all_matched_events(krbn::device_id(1), event_spacebar);
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == true);

  pressed_physical_keys_counter.emplace_back_event(krbn::device_id(1), event_button4);
  REQUIRE(pressed_physical_keys_counter.is_pointing_button_pressed(krbn::device_id(1)) == true);

  pressed_physical_keys_counter.erase_all_matched_events(krbn::device_id(1), event_button4);
  REQUIRE(pressed_physical_keys_counter.is_pointing_button_pressed(krbn::device_id(1)) == false);

  // ----------------------------------------
  // Push twice

  pressed_physical_keys_counter.emplace_back_event(krbn::device_id(1), event_spacebar);
  pressed_physical_keys_counter.emplace_back_event(krbn::device_id(1), event_spacebar);
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == false);

  pressed_physical_keys_counter.erase_all_matched_events(krbn::device_id(1), event_spacebar);
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == true);

  // ----------------------------------------
  // Mixed values

  pressed_physical_keys_counter.emplace_back_event(krbn::device_id(1), event_spacebar);
  pressed_physical_keys_counter.emplace_back_event(krbn::device_id(1), event_tab);
  pressed_physical_keys_counter.emplace_back_event(krbn::device_id(1), event_button1);
  pressed_physical_keys_counter.emplace_back_event(krbn::device_id(1), event_button2);
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == false);

  pressed_physical_keys_counter.erase_all_matched_events(krbn::device_id(1), event_spacebar);
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == false);

  pressed_physical_keys_counter.erase_all_matched_events(krbn::device_id(1), event_tab);
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == false);

  pressed_physical_keys_counter.erase_all_matched_events(krbn::device_id(1), event_button1);
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == false);

  pressed_physical_keys_counter.erase_all_matched_events(krbn::device_id(1), event_button2);
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == true);

  // ----------------------------------------
  // Erase by device_id

  pressed_physical_keys_counter.emplace_back_event(krbn::device_id(1), event_spacebar);
  pressed_physical_keys_counter.emplace_back_event(krbn::device_id(1), event_tab);
  pressed_physical_keys_counter.emplace_back_event(krbn::device_id(1), event_button1);
  pressed_physical_keys_counter.emplace_back_event(krbn::device_id(1), event_button2);
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == false);

  pressed_physical_keys_counter.erase_all_matched_events(krbn::device_id(1));
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == true);
}

TEST_CASE("pressed_physical_keys_counter.usage_page") {
  krbn::pressed_physical_keys_counter pressed_physical_keys_counter;
  krbn::event_queue event_queue;

  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == true);
  REQUIRE(pressed_physical_keys_counter.is_pointing_button_pressed(krbn::device_id(1)) == false);

  event_queue.emplace_back_event(krbn::device_id(1),
                                 100,
                                 krbn::hid_usage_page(kHIDPage_KeyboardOrKeypad),
                                 krbn::hid_usage(kHIDUsage_KeyboardTab),
                                 1);
  REQUIRE(pressed_physical_keys_counter.update(event_queue.get_events().back()) == true);
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == false);
  REQUIRE(pressed_physical_keys_counter.is_pointing_button_pressed(krbn::device_id(1)) == false);

  event_queue.emplace_back_event(krbn::device_id(1),
                                 200,
                                 krbn::hid_usage_page(kHIDPage_KeyboardOrKeypad),
                                 krbn::hid_usage(kHIDUsage_KeyboardTab),
                                 0);
  REQUIRE(pressed_physical_keys_counter.update(event_queue.get_events().back()) == true);
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == true);
  REQUIRE(pressed_physical_keys_counter.is_pointing_button_pressed(krbn::device_id(1)) == false);

  event_queue.emplace_back_event(krbn::device_id(1),
                                 300,
                                 krbn::hid_usage_page(kHIDPage_Button),
                                 krbn::hid_usage(1),
                                 1);
  REQUIRE(pressed_physical_keys_counter.update(event_queue.get_events().back()) == true);
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == false);
  REQUIRE(pressed_physical_keys_counter.is_pointing_button_pressed(krbn::device_id(1)) == true);

  event_queue.emplace_back_event(krbn::device_id(1),
                                 400,
                                 krbn::hid_usage_page(kHIDPage_Button),
                                 krbn::hid_usage(1),
                                 0);
  REQUIRE(pressed_physical_keys_counter.update(event_queue.get_events().back()) == true);
  REQUIRE(pressed_physical_keys_counter.empty(krbn::device_id(1)) == true);
  REQUIRE(pressed_physical_keys_counter.is_pointing_button_pressed(krbn::device_id(1)) == false);
}

int main(int argc, char* const argv[]) {
  krbn::thread_utility::register_main_thread();
  return Catch::Session().run(argc, argv);
}
