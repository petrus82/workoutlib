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

  /**
   * @brief Set the number of times the IntervalIterator will loop over the
   * sequence (Interval - Vector of subIntervals) before reaching the sentinel.
   * Minimum is 1, maximum is INT_MAX.
   *
   * @param repeats
   */
  void setRepeats (int repeats)
  {
    if (repeats >= 1)
      {
        m_repeats = repeats;
      }
  }
  int getRepeats () const { return m_repeats; }

  void addSubInterval (Interval &&interval)
  { m_intervals.emplace_back (std::move (interval)); }

  voidReturn removeSubInterval (std::size_t index) noexcept
  {
    if (index >= m_intervals.size ())
      {
        return std::unexpected (std::format (
            "No element with index {} exists. Cannot remove it.", index));
      }
    m_intervals.erase (m_intervals.cbegin ()
                       + static_cast<std::ptrdiff_t> (index));
    return {};
  }

  struct Sentinel
  {
  };

  struct IntervalIterator
  {
    explicit IntervalIterator (Interval &parent,
                               std::span<Interval> subIntervals) noexcept
        : m_parent (parent), m_subIntervals (subIntervals)
    {
    }
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Interval;

    Interval &getInterval ()
    {
      if (m_index >= std::ssize (m_subIntervals)
          && m_repeats >= m_parent.m_repeats)
        {
          throw std::out_of_range ("Iterator out of range.");
        }
      if (m_index == PARENT_INDEX)
        {
          return m_parent;
        }
      return m_subIntervals[m_index];
    }
    // Throws std::out_of_range
    Interval &operator* () { return getInterval (); }
    // Throws std::out_of_range
    Interval *operator->() { return &getInterval (); }

    IntervalIterator &operator++ () noexcept
    {
      ++m_index;
      if (m_index >= m_subIntervals.size ())
        {
          ++m_repeats;
          if (m_repeats <= m_parent.m_repeats)
            {
              m_index = PARENT_INDEX;
            }
        }
      return *this;
    }

    IntervalIterator operator++ (int) noexcept
    {
      auto prev = *this;
      ++*this;
      return prev;
    }

    bool operator== (const Sentinel sentinel) const noexcept
    {
      return m_index >= m_subIntervals.size ()
             && m_repeats > m_parent.m_repeats;
    }

  private:
    // NOLINTNEXTLINE
    Interval &m_parent;
    std::span<Interval> m_subIntervals;
    static constexpr int PARENT_INDEX{ -1 };
    long m_index{ PARENT_INDEX };
    int m_repeats{ 1 };
  };

  auto begin () { return IntervalIterator (*this, m_intervals); }
  static auto end () { return Sentinel{}; }
  auto count () const { return m_intervals.size (); }
  auto subIntervalAt (std::size_t index) { return m_intervals.at (index); }

private:
  DurationT m_duration{};
  IntensityT m_intensity;

  Intervals m_intervals;
  int m_repeats{ 1 };
};

} // namespace Workouts