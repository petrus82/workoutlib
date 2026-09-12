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

  template <bool IsConst> struct IntervalIteratorBase {
  public:
    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Interval;
    using pointer = std::conditional_t<IsConst, const Interval *, Interval *>;
    using reference = std::conditional_t<IsConst, const Interval &, Interval &>;

    IntervalIteratorBase() noexcept = default;

    using ParentPtr = std::conditional_t<IsConst, const Interval *, Interval *>;
    using SubIntervalSpan =
        std::conditional_t<IsConst, std::span<const Interval>,
                           std::span<Interval>>;
    using RepeatSpan =
        std::conditional_t<IsConst, std::span<const Repeat>, std::span<Repeat>>;

    explicit IntervalIteratorBase(ParentPtr parent,
                                  difference_type pos = 0) noexcept
        : m_parent(parent),
          m_subIntervals(parent != nullptr
                             ? SubIntervalSpan(parent->m_intervals)
                             : SubIntervalSpan{}),
          m_repeats(parent != nullptr ? RepeatSpan(parent->m_repeats)
                                      : RepeatSpan{}),
          m_pos(pos) {}

    template <bool OtherConst>
      requires(IsConst && !OtherConst)
    IntervalIteratorBase(const IntervalIteratorBase<OtherConst> &other) noexcept
        : m_parent(other.m_parent), m_subIntervals(other.m_subIntervals),
          m_repeats(other.m_repeats), m_pos(other.m_pos) {}

    static difference_type count(const Interval &parent) noexcept {
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

    [[nodiscard]] reference at(difference_type index) const {
      if (m_parent == nullptr) {
        throw std::out_of_range("Iterator is value-initialized/singular.");
      }
      const difference_type total = count();
      if (index < 0 || index >= total) {
        throw std::out_of_range("Iterator index out of range.");
      }

      if (m_repeats.empty()) {
        if (index == 0) {
          return *m_parent;
        }
        return m_subIntervals[index - 1];
      }

      // Precompute count per level
      std::vector<difference_type> levelCounts(m_repeats.size(), 0);
      difference_type level{0};
      for (const auto &repeat : m_repeats) {
        const difference_type segLen = 1 + repeat.end - repeat.begin;
        if (level < 1) {
          levelCounts[level] = segLen * repeat.times;
        } else {
          levelCounts[level] =
              levelCounts[level - 1] * repeat.times + segLen * repeat.times;
        }
        ++level;
      }

      difference_type currPos = index;
      for (auto lvl = std::ssize(m_repeats) - 1; lvl >= 0; --lvl) {
        const auto &repeat = m_repeats[lvl];
        const difference_type segLen = 1 + repeat.end - repeat.begin;
        if (lvl == 0) {
          const difference_type withinIter = currPos % segLen;
          const difference_type targetIndex = repeat.begin + withinIter;
          if (targetIndex == PARENT_INDEX) {
            return *m_parent;
          }
          return m_subIntervals[targetIndex];
        }

        const difference_type prevTotal = levelCounts[lvl - 1];
        const difference_type oneCycle = prevTotal + segLen;
        const difference_type withinCycle = currPos % oneCycle;
        if (withinCycle < prevTotal) {
          currPos = withinCycle;
        } else {
          const difference_type withinSeg = withinCycle - prevTotal;
          const difference_type targetIndex = repeat.begin + withinSeg;
          if (targetIndex == PARENT_INDEX) {
            return *m_parent;
          }
          return m_subIntervals[targetIndex];
        }
      }

      return *m_parent;
    }

    reference operator*() const { return at(m_pos); }
    pointer operator->() const { return &at(m_pos); }
    reference operator[](difference_type n) const { return at(m_pos + n); }

    IntervalIteratorBase &operator++() noexcept {
      ++m_pos;
      return *this;
    }

    IntervalIteratorBase operator++(int) noexcept {
      IntervalIteratorBase prev = *this;
      ++m_pos;
      return prev;
    }

    IntervalIteratorBase &operator--() noexcept {
      --m_pos;
      return *this;
    }

    IntervalIteratorBase operator--(int) noexcept {
      IntervalIteratorBase prev = *this;
      --m_pos;
      return prev;
    }

    IntervalIteratorBase &operator+=(difference_type n) noexcept {
      m_pos += n;
      return *this;
    }

    IntervalIteratorBase &operator-=(difference_type n) noexcept {
      m_pos -= n;
      return *this;
    }

    IntervalIteratorBase operator+(difference_type n) const noexcept {
      IntervalIteratorBase res = *this;
      res.m_pos += n;
      return res;
    }

    friend IntervalIteratorBase
    operator+(difference_type n, const IntervalIteratorBase &it) noexcept {
      return it + n;
    }

    IntervalIteratorBase operator-(difference_type n) const noexcept {
      IntervalIteratorBase res = *this;
      res.m_pos -= n;
      return res;
    }

    template <bool OtherConst>
    difference_type
    operator-(const IntervalIteratorBase<OtherConst> &other) const noexcept {
      return m_pos - other.m_pos;
    }

    template <bool OtherConst>
    bool
    operator==(const IntervalIteratorBase<OtherConst> &other) const noexcept {
      if (m_parent != other.m_parent) {
        return false;
      }
      return m_pos == other.m_pos;
    }

    template <bool OtherConst>
    auto
    operator<=>(const IntervalIteratorBase<OtherConst> &other) const noexcept {
      return m_pos <=> other.m_pos;
    }

  private:
    template <bool> friend struct IntervalIteratorBase;
    ParentPtr m_parent{nullptr};
    SubIntervalSpan m_subIntervals{};
    RepeatSpan m_repeats{};
    static constexpr int PARENT_INDEX{-1};
    difference_type m_pos{0};
  };

  using IntervalIterator = IntervalIteratorBase<false>;
  using ConstIntervalIterator = IntervalIteratorBase<true>;
  using iterator = IntervalIterator;
  using const_iterator = ConstIntervalIterator;

  auto begin(this auto &self) {
    using It = std::conditional_t<
        std::is_const_v<std::remove_reference_t<decltype(self)>>,
        ConstIntervalIterator, IntervalIterator>;
    return It(&self, 0);
  }

  auto end(this auto &self) {
    using It = std::conditional_t<
        std::is_const_v<std::remove_reference_t<decltype(self)>>,
        ConstIntervalIterator, IntervalIterator>;
    return It(&self, self.count());
  }

  ConstIntervalIterator cbegin() const { return begin(); }
  ConstIntervalIterator cend() const { return end(); }

  [[nodiscard]] std::ptrdiff_t count() const {
    return IntervalIterator::count(*this);
  }

  decltype(auto) subIntervalAt(this auto &self, std::size_t index) {
    return self.begin().at(static_cast<std::ptrdiff_t>(index));
  }

  auto getSubIntervals(this auto &self) {
    using SpanT = std::conditional_t<
        std::is_const_v<std::remove_reference_t<decltype(self)>>,
        std::span<const Interval>, std::span<Interval>>;
    return SpanT(self.m_intervals);
  }

private:
  DurationT m_duration{};
  IntensityT m_intensity;

  Intervals m_intervals;
  Repeats m_repeats;
  int m_repeat{1};
};

// Static assertions to enforce std::random_access_iterator and
// std::ranges::random_access_range requirements
static_assert(std::random_access_iterator<Interval::IntervalIterator>);
static_assert(std::random_access_iterator<Interval::ConstIntervalIterator>);
static_assert(std::ranges::random_access_range<Interval>);
static_assert(std::ranges::random_access_range<const Interval>);

} // namespace Workouts