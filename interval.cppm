export module interval;

import config;
import common;
export import intensity;
import std;

namespace Workouts {
export class Interval;

/*
  Enables repeating of SubInterval sequences, like 4 x 12x30/30 Intervals
*/

// TODO: Convert this into a class with setter and getter to check
// if last > begin
export struct Repeat {
  // Cannot store iterators or pointers because they will be invalidated after
  // an element has been added to the vector
  std::ptrdiff_t begin{-1};
  std::ptrdiff_t end{-1};
  unsigned int times{1};
};

export using intervalReturn = std::expected<Interval, std::string>;
export using Intervals = std::vector<Interval>;
export using Repeats = std::vector<Repeat>;

using DurationT = std::chrono::seconds;
using IntensityT = std::unique_ptr<Intensity>;

class Interval {
public:
  Interval() = default;

  explicit Interval(Intensity &&intensity,
                    std::chrono::seconds duration) noexcept
      : m_duration(duration),
        m_intensity(std::make_unique<Intensity>(std::move(intensity)))

  {}

  ~Interval() = default;
  Interval(const Interval &copy) noexcept
      : m_duration{copy.m_duration}, m_repeat{copy.m_repeat} {
    Intensity intensityCopy(*copy.m_intensity);
    m_intensity = std::make_unique<Intensity>(std::move(intensityCopy));
  }

  Interval &operator=(const Interval &copy) noexcept {
    if (this == &copy) {
      return *this;
    }
    m_duration = copy.m_duration;
    m_repeat = copy.m_repeat;
    Intensity intensityCopy(*copy.m_intensity);
    m_intensity = std::make_unique<Intensity>(std::move(intensityCopy));
    return *this;
  }
  Interval(Interval &&other) noexcept
      : m_duration(other.m_duration), m_intensity(std::move(other.m_intensity)),
        m_intervals(std::move(other.m_intervals)), m_repeat(other.m_repeat)

  {}

  Interval &operator=(Interval &&other) noexcept {
    if (this == &other) {
      return *this;
    }

    m_duration = other.m_duration;
    m_repeat = other.m_repeat;
    m_intensity = std::move(other.m_intensity);
    m_intervals = std::move(other.m_intervals);
    return *this;
  }

  template <class Rep, class Period>
  constexpr void
  setDuration(const std::chrono::duration<Rep, Period> &duration) noexcept {
    m_duration = std::chrono::duration_cast<DurationT>(duration);
  }

  constexpr DurationT getDuration() const noexcept { return m_duration; }

  void setIntensity(Intensity &&intensity) noexcept {
    // use intensity if there is none already or if intensity is an
    // IntensityPair
    if (!m_intensity || intensity.hasPair()) {
      m_intensity = std::make_unique<Intensity>(std::move(intensity));
      return;
    }

    // If there is already an intensity, maybe the intensity has already a
    // Level::Low intensity and we want to set Level::High intensity now, so we
    // cannot just overwrite it
    m_intensity->setTarget(intensity.getTarget(intensity.getLevel()),
                           intensity.getType(), intensity.getLevel());
  }

  Intensity &getIntensity() { return *m_intensity; }
  const Intensity &getIntensity() const { return *m_intensity; }

  /**
   * @brief Set the number of times the IntervalIterator will loop over the
   * sequence (Interval - Vector of subIntervals) before reaching the sentinel.
   * Minimum is 1, maximum is INT_MAX.
   *
   * @param repeats
   */
  void setRepeats(int repeats) {
    if (repeats >= 1) {
      m_repeat = repeats;
    }
  }
  int getRepeats() const { return m_repeat; }

  // throws std::runtime_error if preconditions are violated.
  void addRepeat(Repeat repeats) {
    // Preconditions
    if (repeats.end < -1) {
      throw std::runtime_error("Repeat::end cannot be less than the parent "
                               "interval index of -1.");
    }
    if (repeats.begin < -1) {
      throw std::runtime_error("Repeat::begin cannot be less than the "
                               "parent interval index of -1.");
    }
    if (repeats.begin > repeats.end) {
      throw std::runtime_error("Don't construct a repeat with a start "
                               "value above the end value.");
    }
    if (repeats.begin > static_cast<std::ptrdiff_t>(m_intervals.size() - 1)) {
      throw std::runtime_error("Repeat::begin is above the valid index range.");
    }
    if (repeats.end > (static_cast<std::ptrdiff_t>(m_intervals.size() - 1))) {
      throw std::runtime_error("Repeat::end is above the valid index range.");
    }
    if (repeats.times == 0) {
      throw std::runtime_error("Repeat::times must be at least 1.");
    }
    m_repeats.emplace_back(repeats);
  }

  void removeRepeat(std::ptrdiff_t index) {
    if (index >= static_cast<std::ptrdiff_t>(m_repeats.size()) || index < 0) {
      throw std::runtime_error(
          std::format("There is no repeat with an index of {}.", index));
    }
    m_repeats.erase(m_repeats.begin() + index);
  }

  std::ptrdiff_t addSubInterval(Interval &&interval) {
    auto index{static_cast<std::ptrdiff_t>(m_intervals.size())};
    m_intervals.emplace_back(std::move(interval));
    return index;
  }

  voidReturn removeSubInterval(std::size_t index) noexcept {
    if (index >= m_intervals.size()) {
      return std::unexpected(std::format(
          "No element with index {} exists. Cannot remove it.", index));
    }
    m_intervals.erase(m_intervals.cbegin() +
                      static_cast<std::ptrdiff_t>(index));
    return {};
  }

  struct IntervalIterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Interval;
    using pointer = Interval *;
    using reference = Interval &;

    IntervalIterator() noexcept = default;

    explicit IntervalIterator(Interval &parent) noexcept
        : m_parent(&parent), m_subIntervals(parent.m_intervals),
          m_repeats(parent.m_repeats) {
      m_counts.reserve(m_repeats.size());
      m_counts = std::vector<std::ptrdiff_t>(m_repeats.size(), 0);

      if (!m_repeats.empty()) {
        m_index = m_repeats[0].begin;
      }
    }

  private:
    struct TerminalTag {};

    IntervalIterator(Interval &parent, TerminalTag) noexcept
        : m_parent(&parent), m_subIntervals(parent.m_intervals),
          m_repeats(parent.m_repeats),
          m_level(static_cast<std::ptrdiff_t>(parent.m_repeats.size())) {
      m_counts = std::vector<std::ptrdiff_t>(m_repeats.size(), 0);
      if (!m_repeats.empty()) {
        m_counts.back() = m_repeats.back().times;
        m_index = m_repeats.back().end + 1;
      } else {
        m_index = static_cast<std::ptrdiff_t>(m_subIntervals.size());
      }
    }

    friend class Interval;

  public:

    Interval &getInterval() const {
      if (m_parent == nullptr) {
        throw std::out_of_range("Iterator is value-initialized/singular.");
      }
      if (m_index >= std::ssize(m_subIntervals)) {
        throw std::out_of_range("Iterator out of range.");
      }
      if (m_index == PARENT_INDEX) {
        return *m_parent;
      }
      return m_subIntervals[m_index];
    }

    // Throws std::out_of_range
    Interval &operator*() const { return getInterval(); }
    // Throws std::out_of_range
    Interval *operator->() const { return &getInterval(); }

    IntervalIterator &operator++() noexcept {
      advance();
      return *this;
    }

    IntervalIterator operator++(int) const noexcept {
      IntervalIterator prev = *this;
      advance();
      return prev;
    }

    [[nodiscard]] bool is_terminal() const noexcept {
      if (m_parent == nullptr) {
        return true;
      }
      if (m_repeats.empty()) {
        return m_index >= std::ssize(m_subIntervals);
      }
      return m_level >= std::ssize(m_repeats) && !m_counts.empty() &&
             m_counts.back() >= m_repeats.back().times;
    }

    bool operator==(const IntervalIterator &other) const noexcept {
      if (m_parent != other.m_parent) {
        return false;
      }
      if (m_parent == nullptr) {
        return true;
      }
      const bool this_term = is_terminal();
      const bool other_term = other.is_terminal();
      if (this_term || other_term) {
        return this_term == other_term;
      }
      return m_index == other.m_index && m_level == other.m_level &&
             m_counts == other.m_counts;
    }

  private:
    void advance() const noexcept {
      ++m_index;
      if (!m_repeats.empty() && m_level < std::ssize(m_repeats) &&
          m_index > m_repeats[m_level].end) {
        ++m_counts.at(m_level);
        if (m_counts.at(m_level) >= m_repeats[m_level].times) {
          ++m_level;
          if (m_level < std::ssize(m_repeats)) {
            m_index = m_repeats[m_level].begin;
          }
        } else if (m_level > 0) {
          m_level = 0;
          m_counts.at(0) = 0;
          m_index = m_repeats[0].begin;
        } else {
          m_index = m_repeats[0].begin;
        }
      }
    }

    Interval *m_parent{nullptr};
    std::span<Interval> m_subIntervals;
    std::span<Repeat> m_repeats;
    mutable std::vector<std::ptrdiff_t> m_counts;
    static constexpr int PARENT_INDEX{-1};
    mutable std::ptrdiff_t m_index{PARENT_INDEX};
    mutable std::ptrdiff_t m_level{0};
  };

  IntervalIterator begin() { return IntervalIterator(*this); }
  IntervalIterator end() {
    return IntervalIterator(*this, typename IntervalIterator::TerminalTag{});
  }
  std::ptrdiff_t count() const {
    std::ptrdiff_t nrSubIntervals{};
    std::ptrdiff_t level{0};
    if (m_repeats.size() > 0) {
      for (const auto &repeat : m_repeats) {
        if (level++ < 1) {
          nrSubIntervals
              // subInterval sequence
              // +1 because index start at 0
              // (if there is only one element, this has index 0)
              = (1 + repeat.end - repeat.begin) * repeat.times;
        } else {
          // the number of intervals in the first sequence
          nrSubIntervals *= repeat.times;

          // + the sequence in this level
          nrSubIntervals += (1 + repeat.end - repeat.begin) * repeat.times;
        }
      }
      return nrSubIntervals;
    }
    // Number of subIntervals + parent interval if there is no repeat
    return static_cast<std::ptrdiff_t>(m_intervals.size() + 1);
  }
  auto subIntervalAt(std::size_t index) { return m_intervals.at(index); }

  std::span<Interval> getSubIntervals() { return m_intervals; }

private:
  DurationT m_duration{};
  IntensityT m_intensity;

  Intervals m_intervals;
  Repeats m_repeats;
  int m_repeat{1};
};

// Static assertions to enforce std::forward_iterator and std::ranges::forward_range requirements
static_assert(std::forward_iterator<Interval::IntervalIterator>);
static_assert(std::ranges::forward_range<Interval>);

} // namespace Workouts