#pragma once

#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <queue>

namespace tedlhy::minekraf {

using EventCategoryID = ssize_t;
using EventID = size_t;

/**
 * int eventCategoryHandler(EventID id, void* eventdata, void* categorydata);
 *
 * May call EventQueue::push_event() to push error event onto the queue.
 *
 * Returns 0 if no error occured.
 */
using EventHandlerFunc = int (*)(EventID, void*, void*);

struct EventCategory {
  EventCategoryID id;
  std::string name;
  std::set<EventID> event_ids;
  std::vector<EventHandlerFunc> handlers;
};

class EventQueue {
  struct Event {
    void* data = nullptr;
    bool free_data_after_use = false;
  };

  struct EventCategoryInternal {
    EventCategory category;
    void* data = nullptr;
    bool free_data_after_use = false;
  };

  std::map<EventCategoryID, EventCategoryInternal> registry;
  std::queue<std::pair<EventID, Event>> queue;
  std::mutex mutex;

  /// see push_event()
  void _push_event_nolock(EventID id, void* data, size_t data_size);

  /// see find_categories()
  std::vector<EventCategoryID> _find_categories_nolock(EventID event_id);

public:
  static EventQueue& get();

  EventQueue();
  ~EventQueue();

  /**
   * Push event onto the queue.
   *
   * On the next tick, the queue will be processed, but the handling of this
   * event may not happen in that next tick.
   *
   * Pass `data_size` > 0 to tell the EventQueue to copy the value of data and
   * free the memory of the copied data pointer after the event has been
   * processed and popped from the queue.
   */
  void push_event(EventID id, void* data, size_t data_size);

  /**
   * Tick event queue, call event handlers.
   *
   * This method returns the count of events handled in this tick.
   */
  size_t tick();
  size_t tick(size_t timeout_ms);

  /**
   * Insert category into event registry.
   *
   * This method returns true if insertion was successful, false otherwise.
   */
  bool insert_category(EventCategory&& category);

  /**
   * Insert category into event registry (with user provided data).
   *
   * Pass `data_size` > 0 to tell the EventQueue to copy the value of data and
   * free the memory of the copied data pointer after the category is removed.
   *
   * This method returns true if insertion was successful, false otherwise.
   */
  bool insert_category(EventCategory&& category, void* data, size_t data_size);

  /**
   * Remove category from event registry.
   *
   * It also removes all event handlers registered with the category.
   *
   * This method returns true if removal was successful, false otherwise.
   */
  bool remove_category(EventCategoryID id);

  /**
   * Find categories by event id.
   *
   * This method returns all category ids which watch for the event event_id.
   */
  std::vector<EventCategoryID> find_categories(EventID event_id);

  /**
   * Find category by name.
   *
   * This method returns an optional of the found category id, or empty otherwise.
   */
  std::optional<EventCategoryID> find_category(std::string_view name);

  /**
   * Finds the next available category id.
   *
   * This method returns the next closest integer id to the `start` id, which
   * does not have a category registered.
   */
  EventCategoryID find_next_free_category(EventCategoryID start);

  /**
   * Append event id to the category's watch list.
   *
   * This method returns true if appending was successful, false otherwise.
   */
  bool append_category_event(EventCategoryID id, EventID event_id);

  /**
   * Remove event id from the category's watch list.
   *
   * This method returns true if removal was successful, false otherwise.
   */
  bool remove_category_event(EventCategoryID id, EventID event_id);

  /**
   * Register a new event handler with event category.
   *
   * Note: It does not replace the current event handler, but it appends to a
   * list of handlers to call when an event from this category fires.
   *
   * This method returns the index of the handler, or `SIZE_MAX` (or equivalent)
   * if there was an error.
   */
  size_t register_handler(EventCategoryID id, EventHandlerFunc handler);

  /**
   * Remove an event handler from an event category.
   *
   * This method returns true if removal was successful, false otherwise.
   */
  bool remove_handler(EventCategoryID id, size_t index);

  /**
   * Remove event handlers from an event category.
   *
   * This method returns true if removal was successful, false otherwise.
   */
  bool remove_handlers(EventCategoryID id);

  /**
   * Return last handler from an event category, and remove it from that
   * category.
   */
  EventHandlerFunc pop_handler(EventCategoryID id);

  /**
   * Return handler from an event category, and remove if from that category.
   */
  EventHandlerFunc pop_handler(EventCategoryID id, size_t index);
};

}  // namespace tedlhy::minekraf
