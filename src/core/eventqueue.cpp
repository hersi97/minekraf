#include "eventqueue.h"

#include <chrono>
#include <limits>

#include "logger/logger.h"

using namespace tedlhy::minekraf;

constexpr EventID EVENTQUEUEERROR_ID{std::numeric_limits<EventID>::max()};
constexpr EventCategoryID EVENTQUEUEERROR_CATEGORYID{std::numeric_limits<EventCategoryID>::max()};

enum EventQueueErrorReason {
  NoReason,
  EventCategoryNotFound,
  HandlerReturnError,
  QueueNotEmpty,
  QueueNotEmptyNoCategory,
  _EventQueueErrorReason_size,
};

struct EventQueueError {
  EventCategory category;
  EventQueueErrorReason reason;
  EventID queue_id = 0;
  int handler_return = 0;
};

int eventqueue_error_handler(EventID id, void* data, void* categorydata)
{
  constexpr const char tag[] = "EventQueueError";

  auto& logger = logger::get();

  if (id != EVENTQUEUEERROR_ID) {
    logger.critical("EventQueueError handler called with invalid event id! THIS SHOULD NOT HAPPEN");
    throw std::runtime_error("EventQueueError handler called with invalid event id");
  }

  auto error = static_cast<EventQueueError*>(data);

  if (!error) {
    logger.critical("Failure to cast error event data! THIS SHOULD NOT HAPPEN");
    throw std::runtime_error("Failure to cast error event data");
  }

  logger.trace("{}: {{ error {}, Category {} }}", tag, static_cast<int>(error->reason), error->category.name);

  switch (error->reason) {
    case NoReason:
      logger.error("{}: Event (id {}) handling general failure", tag, error->queue_id);
      break;
    case EventCategoryNotFound:
      logger.warning("{}: Event id {} is not registered in a Category", tag, error->queue_id);
      break;
    case HandlerReturnError:
      logger.error("{}: Category {}->handler({}, ...) returned with error code: {}", tag, error->category.name,
        error->category.id, error->handler_return);
      break;
    case QueueNotEmpty:
      logger.warning("{}: EventQueue is being destructed, but queue still contains Category {} Event id {}", tag,
        error->category.name, error->queue_id);
      break;
    case QueueNotEmptyNoCategory:
      logger.warning("{}: EventQueue is being destructed, but queue still contains Event id {}", tag, error->queue_id);
      break;
    default:
      using reason_t = std::underlying_type<EventQueueErrorReason>::type;
      logger.error("{}: event handler called with unknown reason {}", tag, static_cast<reason_t>(error->reason));
      return error->reason;
  };

  return 0;
}

EventQueue& EventQueue::get()
{
  static EventQueue eventqueue;

  return eventqueue;
}

EventQueue::EventQueue() : registry(), queue()
{
  EventCategoryInternal errorcat{
    {
      .id = EVENTQUEUEERROR_CATEGORYID,
      .name = "EventQueueError",
      .event_ids = {EVENTQUEUEERROR_ID},
      .handlers = {eventqueue_error_handler},
    },
    nullptr,
    false,
  };
  registry.emplace(EVENTQUEUEERROR_CATEGORYID, std::move(errorcat));
}

EventQueue::~EventQueue()
{
  while (!queue.empty()) {
    const std::lock_guard lock(mutex);

    auto [id, event] = queue.front();
    queue.pop();

    auto categoryids = _find_categories_nolock(id);
    if (categoryids.empty()) {
      // event id has no category anymore
      EventQueueError error{
        .category = {},
        .reason = QueueNotEmptyNoCategory,
        .queue_id = id,
      };
      eventqueue_error_handler(EVENTQUEUEERROR_ID, &error, nullptr);
    }
    for (const auto& categoryid : categoryids) {
      auto _category_internal = registry.find(categoryid);
      if (_category_internal != registry.end()) {
        EventQueueError error{
          .category = _category_internal->second.category,
          .reason = QueueNotEmpty,
          .queue_id = id,
        };
        eventqueue_error_handler(EVENTQUEUEERROR_ID, &error, _category_internal->second.data);
      }
    }

    if (event.free_data_after_use) {
      // free the copied data (see: push_event())
      std::free(event.data);
    }
  }
}

void EventQueue::_push_event_nolock(EventID id, void* data, size_t data_size)
{
  auto& logger = logger::get();
  logger.trace("push_event(): Event {:#x}", id);

  Event event{data, false};

  if (data && data_size) {
    // store a copy of the data,
    auto handler_data = std::malloc(data_size);
    std::memcpy(handler_data, data, data_size);
    logger.trace("push_event(): malloc {:p}, size: {}", handler_data, data_size);
    event.data = handler_data;
    event.free_data_after_use = true;
  }

  queue.emplace(id, std::move(event));
}

void EventQueue::push_event(EventID id, void* data, size_t data_size)
{
  std::lock_guard lock(mutex);

  _push_event_nolock(id, data, data_size);
}

size_t EventQueue::tick()
{
  constexpr auto max_timeout = std::chrono::milliseconds::max().count();
  return tick(max_timeout);
}

size_t EventQueue::tick(size_t timeout_ms)
{
  using namespace std::chrono;

  size_t count = 0;
  auto timeout = milliseconds{timeout_ms};

  auto timepoint = steady_clock::now();

  auto& logger = logger::get();

  while (!queue.empty()) {
    const std::lock_guard lock(mutex);

    auto [id, event] = queue.front();
    queue.pop();
    logger.trace("EventQueue::tick(): pop {{ {:#x}, {:p} }}", id, static_cast<void*>(event.data));

    EventCategory* category = nullptr;
    for (auto& [_, _category_internal] : registry) {
      logger.trace("EventQueue::tick(): - Category {} (id {})", _category_internal.category.name,
        _category_internal.category.id);
      auto& _category = _category_internal.category;
      // find category by event id
      if (!_category.event_ids.contains(id)) {
        continue;
      }
      category = &_category;
      logger.trace("EventQueue::tick(): - Event {:#x} <-> Category {} (id {})", id, category->name, category->id);
      for (auto handler : category->handlers) {
        logger.trace("EventQueue::tick():   - call handler {:p} for Category {} (id {})", (void*)handler,
          category->name, category->id);
        auto err = handler(id, event.data, _category_internal.data);
        if (err) {
          EventQueueError error{
            .category = *category,
            .reason = HandlerReturnError,
            .handler_return = err,
          };
          _push_event_nolock(EVENTQUEUEERROR_ID, &error, sizeof(error));
        }
      }
      count++;
    }

    if (!category) {
      logger.trace("EventQueue::tick(): Category not found for event {:#x}", id);
      if (id == EVENTQUEUEERROR_ID) {
        // no EventQueueError category?
        // oh no, how did you manage this?
        throw std::runtime_error("EventQueueError category is not registered");
      }
      EventQueueError error{
        .category = {},
        .reason = EventCategoryNotFound,
        .queue_id = id,
      };
      _push_event_nolock(EVENTQUEUEERROR_ID, &error, sizeof(error));
    }

    if (event.free_data_after_use) {
      // free the copied data (see: push_event())
      logger.trace("EventQueue::tick(): free {:p}", event.data);
      std::free(event.data);
    }

    if (auto delta = steady_clock::now() - timepoint; duration_cast<milliseconds>(delta) > timeout) {
      logger.trace("EventQueue::tick(): timeout after {}ns", duration_cast<nanoseconds>(delta).count());
      break;
    }
  }
  return count;
}

bool EventQueue::insert_category(EventCategory&& category)
{
  return insert_category(std::move(category), nullptr, 0);
}

bool EventQueue::insert_category(EventCategory&& category, void* data, size_t data_size)
{
  const std::lock_guard lock(mutex);

  auto& logger = logger::get();

  for (auto& [_, _category_internal] : registry) {
    auto& _category = _category_internal.category;
    if (_category.name == category.name) {
      logger.warning(
        "Category {} (id {}) has the same name as Category {} (id {}), "
        "finding Category {} by name will be unpredictable, consider using another name",
        category.name, category.id, _category.name, _category.id, category.name);
      break;
    }
  }

  EventCategoryInternal category_internal{category, data, false};

  if (data && data_size) {
    // store a copy of the data,
    auto categorydata = std::malloc(data_size);
    std::memcpy(categorydata, data, data_size);
    category_internal.data = categorydata;
    category_internal.free_data_after_use = true;
  }

  auto [_, success] = registry.emplace(category.id, std::move(category_internal));
  return success;
}

bool EventQueue::remove_category(EventCategoryID id)
{
  const std::lock_guard lock(mutex);

  auto _category_internal = registry.find(id);
  if (_category_internal != registry.end()) {
    if (_category_internal->second.free_data_after_use) {
      // see: insert_category()
      std::free(_category_internal->second.data);
    }
    registry.erase(_category_internal);
    return true;
  }

  return false;
}

std::vector<EventCategoryID> EventQueue::_find_categories_nolock(EventID event_id)
{
  std::vector<EventCategoryID> categories;
  for (const auto& [id, category_internal] : registry) {
    auto& category = category_internal.category;
    if (category.event_ids.find(event_id) != category.event_ids.end()) {
      categories.push_back(id);
    }
  }
  return categories;
}

std::vector<EventCategoryID> EventQueue::find_categories(EventID event_id)
{
  const std::lock_guard lock(mutex);

  return _find_categories_nolock(event_id);
}

std::optional<EventCategoryID> EventQueue::find_category(std::string_view name)
{
  const std::lock_guard lock(mutex);

  for (const auto& [id, category_internal] : registry) {
    auto& category = category_internal.category;
    if (category.name == name) {
      return id;
    }
  }
  return {};
}

EventCategoryID EventQueue::find_next_free_category(EventCategoryID start)
{
  const std::lock_guard lock(mutex);

  auto _category = registry.find(start);
  // https://en.cppreference.com/w/cpp/container/map
  // keys are sorted by std::less<KeyT>, so we can iterate without calling find
  // on every iteration
  while (_category != registry.end() && start == _category->first) {
    start++;
    _category++;
  }

  return start;
}

bool EventQueue::append_category_event(EventCategoryID id, EventID event_id)
{
  const std::lock_guard lock(mutex);

  auto _category_internal = registry.find(id);
  if (_category_internal == registry.end()) {
    return false;
  }

  auto& category = _category_internal->second.category;
  auto [_, success] = category.event_ids.insert(event_id);
  return success;
}

bool EventQueue::remove_category_event(EventCategoryID id, EventID event_id)
{
  const std::lock_guard lock(mutex);

  auto _category_internal = registry.find(id);
  if (_category_internal == registry.end()) {
    return false;
  }

  auto& category = _category_internal->second.category;
  return category.event_ids.erase(event_id);
}

size_t EventQueue::register_handler(EventCategoryID id, EventHandlerFunc handler)
{
  const std::lock_guard lock(mutex);

  auto& logger = logger::get();

  auto _category_internal = registry.find(id);
  if (_category_internal == registry.end()) {
    logger.error("EventQueue::register_handler(): Category id {} does not exist", id);
    return std::numeric_limits<size_t>::max();
  }

  auto& handlers = _category_internal->second.category.handlers;
  handlers.push_back(std::move(handler));
  return handlers.size();
}

bool EventQueue::remove_handler(EventCategoryID id, size_t index)
{
  const std::lock_guard lock(mutex);

  auto& logger = logger::get();

  auto _category_internal = registry.find(id);
  if (_category_internal == registry.end()) {
    logger.error("EventQueue::remove_handler(): Category id {} does not exist", id);
    return false;
  }

  auto& handlers = _category_internal->second.category.handlers;
  if (index >= handlers.size()) {
    logger.error("EventQueue::remove_handler(): Handler index {} is out-of-range", index);
    return false;
  }
  handlers.erase(handlers.begin() + index);
  return true;
}

bool EventQueue::remove_handlers(EventCategoryID id)
{
  const std::lock_guard lock(mutex);

  auto& logger = logger::get();

  auto _category_internal = registry.find(id);
  if (_category_internal == registry.end()) {
    logger.error("EventQueue::remove_handlers(): Category id {} does not exist", id);
    return false;
  }

  auto& handlers = _category_internal->second.category.handlers;
  handlers.clear();
  return true;
}

EventHandlerFunc EventQueue::pop_handler(EventCategoryID id)
{
  const std::lock_guard lock(mutex);

  auto& logger = logger::get();

  auto _category_internal = registry.find(id);
  if (_category_internal == registry.end()) {
    logger.error("EventQueue::pop_handler(): Category id {} does not exist", id);
    return nullptr;
  }

  auto& handlers = _category_internal->second.category.handlers;
  auto handler = handlers.back();
  handlers.erase(handlers.end());
  return handler;
}

EventHandlerFunc EventQueue::pop_handler(EventCategoryID id, size_t index)
{
  const std::lock_guard lock(mutex);

  auto& logger = logger::get();

  auto _category_internal = registry.find(id);
  if (_category_internal == registry.end()) {
    logger.error("EventQueue::pop_handler(): Category id {} does not exist", id);
    return nullptr;
  }

  auto& handlers = _category_internal->second.category.handlers;
  if (index >= handlers.size()) {
    logger.error("EventQueue::pop_handler(): Handler index {} is out-of-range", index);
    return nullptr;
  }
  auto handler = handlers.at(index);
  handlers.erase(handlers.begin() + index);
  return handler;
}
