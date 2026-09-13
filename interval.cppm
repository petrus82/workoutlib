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

  template <class T = Interval> class IntervalIterator {

  public:
    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer_type = T *;
    using reference_type = T &;

    IntervalIterator() noexcept = default;

    explicit IntervalIterator(T *parent, difference_type pos = 0) noexcept
        : m_parent(parent),
          m_subIntervals(parent != nullptr ? (parent->m_intervals)
                                           : std::span<T>{}),
          m_repeats(parent != nullptr ? (parent->m_repeats)
                                      : std::span<const Repeat>{}),
          m_pos(pos) {}

    // Implicit conversion from the non-const to the const iterator,
    // mirroring std::vector::iterator -> std::vector::const_iterator
    template <class U>
      requires std::same_as<T, const Interval> && std::same_as<U, Interval>
    constexpr IntervalIterator(const IntervalIterator<U> &other) noexcept
        : m_parent(other.m_parent), m_subIntervals(other.m_subIntervals),
          m_repeats(other.m_repeats), m_pos(other.m_pos) {}

    static difference_type count(auto &parent) noexcept {
      if (!parent.m_repeats.empty()) {
        difference_type nrSubIntervals{0};
        difference_type level{0};
        for (const auto &repeat : parent.m_repeats) {
          if (level++ < 1) {
            nrSubIntervals = (1 + repeat.end - repeat.begin) * repeat.times;
          } else {
            nrSubIntervals *= repeat.times;
            nrSubIntervals += (1 + repeat.end - repeat.begin) * repeat.times;
          }
        }
        return nrSubIntervals;
      }
      return static_cast<difference_type>(parent.m_intervals.size() + 1);
    }

    [[nodiscard]] difference_type count() const noexcept {
      if (m_parent == nullptr) {
        return 0;
      }
      return count(*m_parent);
    }

    [[nodiscard]] reference_type at(difference_type indexExternal) const {
      if (m_parent == nullptr) {
        throw std::out_of_range("Iterator is value-initialized/singular.");
      }

      if (indexExternal < 0 || indexExternal >= count()) {
        throw std::out_of_range("Iterator index out of range.");
      }

      if (indexExternal == 0) {
        return *m_parent;
      }

      if (m_repeats.empty()) {
        return m_subIntervals[indexExternal - 1];
      }

      // Number of sub Intervals + Parent
      difference_type sequenceLength{std::ssize(m_subIntervals) + 1};

      // The internal index is the modulo division of the external index
      // This eliminates the need to calculate the repeat
      // If an interval has 2 subIntervals (this means a sequenceLength of 3)
      // and a repeat of 2, this yields
      // (1 -> 1), (2 -> 2), (3 -> 3), (4 -> 1), (5 -> 2), (6 -> 3)
      // We have to subtract 1 because the parent interval starts at -1
      // and the index of m_subIntervals starts at 0
      difference_type indexInternal{(indexExternal % sequenceLength) - 1};

      if (indexInternal == PARENT_INDEX) {
        return *m_parent;
      }
      return m_subIntervals[indexInternal];
    }

    reference_type operator*() const { return at(m_pos); }

    pointer_type operator->() const { return &at(m_pos); }
    reference_type operator[](difference_type n) const { return at(m_pos + n); }

    IntervalIterator &operator++() noexcept {
      ++m_pos;
      return *this;
    }

    IntervalIterator operator++(int) noexcept {
      IntervalIterator prev = *this;
      ++m_pos;
      return prev;
    }

    IntervalIterator &operator--() noexcept {
      --m_pos;
      return *this;
    }

    IntervalIterator operator--(int) noexcept {
      IntervalIterator prev = *this;
      --m_pos;
      return prev;
    }

    IntervalIterator &operator+=(difference_type n) noexcept {
      m_pos += n;
      return *this;
    }

    IntervalIterator &operator-=(difference_type n) noexcept {
      m_pos -= n;
      return *this;
    }

    IntervalIterator operator+(difference_type n) const noexcept {
      IntervalIterator res = *this;
      res.m_pos += n;
      return res;
    }

    friend IntervalIterator
    operator+(difference_type n, const IntervalIterator &iterator) noexcept {
      return iterator + n;
    }

    IntervalIterator operator-(difference_type n) const noexcept {
      IntervalIterator res = *this;
      res.m_pos -= n;
      return res;
    }

    // Same-type and cross-instantiation (const/non-const) subtraction;
    // only the IntervalIterator<Interval> and IntervalIterator<const
    // Interval> instantiations are supported.
    template <class U>
      requires(std::same_as<T, Interval> || std::same_as<T, const Interval>) &&
              (std::same_as<U, Interval> || std::same_as<U, const Interval>)
    constexpr difference_type
    operator-(const IntervalIterator<U> &other) const noexcept {
      return m_pos - other.m_pos;
    }

    // Cross-instantiation comparison is only allowed from the non-const to
    // the const iterator; a symmetric member would be ambiguous, while the
    // reverse direction (const == non-const) resolves through the implicit
    // conversion.
    template <class U>
      requires std::same_as<U, T> ||
               (std::same_as<T, Interval> && std::same_as<U, const Interval>)
    constexpr bool operator==(const IntervalIterator<U> &other) const noexcept {
      if (m_parent != other.m_parent) {
        return false;
      }
      return m_pos == other.m_pos;
    }

    auto operator<=>(const IntervalIterator &other) const noexcept {
      return m_pos <=> other.m_pos;
    }

  private:
    // The two intended instantiations need to read each other's state for
    // cross-instantiation comparison, subtraction, and the implicit
    // non-const -> const conversion.
    friend class IntervalIterator<Interval>;
    friend class IntervalIterator<const Interval>;
    T *m_parent{nullptr};
    std::span<T> m_subIntervals;
    std::span<const Repeat> m_repeats;
    static constexpr int PARENT_INDEX{-1};
    difference_type m_pos{0};
  };

  auto begin() { return IntervalIterator(this); }
  auto begin() const { return IntervalIterator(this); }

  auto end() {
    return IntervalIterator(this, IntervalIterator<Interval>::count(*this));
  }
  auto end() const {
    return IntervalIterator(this, IntervalIterator<Interval>::count(*this));
  }

  auto count() const { return IntervalIterator<Interval>::count(*this); }

  auto subIntervalAt(std::size_t index) {
    return IntervalIterator(this).at(static_cast<std::ptrdiff_t>(index));
  }

  auto subIntervalAt(std::size_t index) const {
    return IntervalIterator(this).at(static_cast<std::ptrdiff_t>(index));
  }

  auto getSubIntervals() { return m_intervals; }

private:
  DurationT m_duration{};
  IntensityT m_intensity;

  Intervals m_intervals;
  Repeats m_repeats;
  int m_repeat{1};
};

// Static assertions to enforce std::random_access_iterator and
// std::ranges::random_access_range requirements
static_assert(
    std::random_access_iterator<Interval::IntervalIterator<Interval>>);
static_assert(std::ranges::random_access_range<Interval>);
static_assert(std::ranges::random_access_range<const Interval>);
static_assert(std::convertible_to<Interval::IntervalIterator<Interval>,
                                  Interval::IntervalIterator<const Interval>>);

} // namespace Workouts