export module interval;

import config;
import common;
import intensity;
import std;

namespace Workouts
{
export class Interval;
export using intervalReturn = std::expected<Interval, std::string>;
export using Intervals = std::vector<Interval>;
// export using SubIntervals = std::vector<std::weak_ptr<Interval>>;

using DurationT = std::chrono::seconds;
using IntensityT = std::unique_ptr<Intensity>;

export class Interval
{
public:
  Interval () = default;

  explicit Interval (Intensity &&intensity,
                     std::chrono::seconds duration) noexcept
      : m_intensity (std::make_unique<Intensity> (std::move (intensity))),
        m_duration (duration)
  {
  }

  ~Interval () = default;
  Interval (const Interval &copy) noexcept
      : m_duration{ copy.m_duration }, m_repeats{ copy.m_repeats }
  {
    Intensity intensityCopy (*copy.m_intensity);
    m_intensity = std::make_unique<Intensity> (std::move (intensityCopy));
  }

  Interval &operator= (const Interval &copy) noexcept
  {
    if (this == &copy)
      {
        return *this;
      }
    m_duration = copy.m_duration;
    m_repeats = copy.m_repeats;
    Intensity intensityCopy (*copy.m_intensity);
    m_intensity = std::make_unique<Intensity> (std::move (intensityCopy));
    return *this;
  }
  Interval (Interval &&other) noexcept
      : m_duration (other.m_duration), m_repeats (other.m_repeats),
        m_intensity (std::move (other.m_intensity))
  {
  }

  Interval &operator= (Interval &&other) noexcept
  {
    if (this == &other)
      {
        return *this;
      }

    m_duration = other.m_duration;
    m_repeats = other.m_repeats;
    m_intensity = std::move (other.m_intensity);
    return *this;
  }

  template <class Rep, class Period>
  constexpr void
  setDuration (const std::chrono::duration<Rep, Period> &duration) noexcept
  { m_duration = std::chrono::duration_cast<DurationT> (duration); }

  constexpr DurationT getDuration () const noexcept { return m_duration; }

  void setIntensity (Intensity &&intensity) noexcept
  { m_intensity = std::make_unique<Intensity> (std::move (intensity)); }

  Intensity &getIntensity () { return *m_intensity; }
  const Intensity &getIntensity () const { return *m_intensity; }

  void setRepeats (int repeats) { m_repeats = repeats; }
  int getRepeats () const { return m_repeats; }

  void addSubInterval (Interval &&interval)
  { m_intervals.emplace_back (std::move (interval)); }

  [[deprecated ("Not yet implemented")]] static voidReturn
  removeSubInterval (std::size_t index) noexcept
  { return {}; }

  struct Sentinel
  {
  };

  struct IntervalIterator
  {
    IntervalIterator (Interval &parent, std::span<Interval> subIntervals)
        : m_parent (parent), m_subIntervals (subIntervals)
    {
    }
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Interval;

    Interval &getInterval () const
    {
      if (m_index == PARENT_INDEX)
        {
          return m_parent;
        }
      if (m_index >= m_subIntervals.size ())
        {
          throw std::out_of_range ("Iterator out of range.");
        }
      return m_subIntervals[m_index];
    }
    Interval &operator* () const { return getInterval (); }
    Interval *operator->() const { return &getInterval (); }

    IntervalIterator &operator++ ()
    {
      ++m_index;
      return *this;
    }

    IntervalIterator operator++ (int)
    {
      auto prev = *this;
      ++*this;
      return prev;
    }

    bool operator== (const Sentinel sentinel) const
    { return m_index > m_subIntervals.size (); }

  private:
    // NOLINTNEXTLINE
    Interval &m_parent;
    std::span<Interval> m_subIntervals;
    static constexpr int PARENT_INDEX{ -1 };
    long m_index{ PARENT_INDEX };
  };

  auto begin () { return IntervalIterator (*this, m_intervals); }
  static auto end () { return Sentinel{}; }

private:
  /*   struct IntervalIterator
    {
      using iterator_category = std::random_access_iterator_tag;
      using value_type = Interval;
      using difference_type = std::ptrdiff_t;
      using pointer = Interval *;
      using reference = const Interval &;

      IntervalIterator (pointer parent) : m_parent (parent) {}

      reference operator* () const
      {
        if (m_current_sub_index == 0)
          {
            return *m_parent;
          }
        return m_parent->m_intervals.at (m_current_sub_index);
      }

      pointer operator->()
      {
        if (m_current_sub_index == 0)
          {
            return m_parent;
          }
        if (m_current_repeat > m_parent->m_repeats)
          {
            return nullptr;
          }
        return &m_parent->m_intervals.at (m_current_sub_index - 1);
      }

      Interval *operator++ ()
      {
        if (m_current_sub_index >= m_parent->m_intervals.size ())
          {
            m_current_sub_index = 0;
            ++m_current_repeat;
          }
        if (m_current_repeat == 0)
          {
            return m_parent;
          }
        if (m_current_sub_index < m_parent->m_intervals.size ())
          {
            m_current_sub_index++;
            return &m_parent->m_intervals.at (m_current_sub_index - 1);
          }
        return nullptr;
      }

      // Postfix increment
      IntervalIterator operator++ (auto)
      {
        IntervalIterator tmp = *this;
        ++(*this);
        return tmp;
      }

      struct Sentinel
      {
      };

      friend bool operator== (const IntervalIterator &lhs,
                              const IntervalIterator &rhs)
      { return lhs.m_parent == rhs.m_parent; };

      friend bool operator!= (const IntervalIterator &lhs,
                              const IntervalIterator &rhs)
      { return lhs.m_parent != rhs.m_parent; };

      reference operator[] (std::size_t n) const
      {
        if (n >= m_total_size)
          {
            throw std::out_of_range (
                "Iterator index out of bounds for IntervalIterator.");
          }
        return m_parent->m_intervals.at (n);
      }

      IntervalIterator &operator++ ()
      {
        m_current_repeat++;
        if (m_current_repeat >= m_ptr.m_repeats)
          {
            m_current_repeat = 0;
            m_current_sub_index++;
            m_current_sub_index
                = std::min (m_current_sub_index, m_ptr.m_intervals.size ());
          }
        return *this;
      } */

  /* bool operator== (const IntervalIterator &other) const
  {
    return m_current_sub_index == other.m_current_sub_index
           && m_current_repeat == other.m_current_repeat;
  }

  bool operator!= (const IntervalIterator &other) const
  { return !(*this == other); }

private:
  pointer m_parent;
  std::size_t m_current_sub_index{};
  int m_current_repeat{};
  std::size_t m_total_size{};
}; */

private:
  DurationT m_duration{};
  IntensityT m_intensity;

  Intervals m_intervals;
  int m_repeats{ 1 };
};

} // namespace Workouts