export module interval;

import config;
import common;
export import intensity;
import std;

namespace Workouts
{
export class Interval;

struct Repeat
{
  std::vector<Interval>::const_iterator begin;
  std::vector<Interval>::const_iterator last;
  int times{};
};

export using intervalReturn = std::expected<Interval, std::string>;
export using Intervals = std::vector<Interval>;
export using Repeats = std::vector<Repeat>;

using DurationT = std::chrono::seconds;
using IntensityT = std::unique_ptr<Intensity>;

class Interval
{
public:
  Interval () = default;

  explicit Interval (Intensity &&intensity,
                     std::chrono::seconds duration) noexcept
      : m_duration (duration),
        m_intensity (std::make_unique<Intensity> (std::move (intensity)))

  {
  }

  ~Interval () = default;
  Interval (const Interval &copy) noexcept
      : m_duration{ copy.m_duration }, m_repeat{ copy.m_repeat }
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
    m_repeat = copy.m_repeat;
    Intensity intensityCopy (*copy.m_intensity);
    m_intensity = std::make_unique<Intensity> (std::move (intensityCopy));
    return *this;
  }
  Interval (Interval &&other) noexcept
      : m_duration (other.m_duration),
        m_intensity (std::move (other.m_intensity)),
        m_intervals (std::move (other.m_intervals)), m_repeat (other.m_repeat)

  {
  }

  Interval &operator= (Interval &&other) noexcept
  {
    if (this == &other)
      {
        return *this;
      }

    m_duration = other.m_duration;
    m_repeat = other.m_repeat;
    m_intensity = std::move (other.m_intensity);
    m_intervals = std::move (other.m_intervals);
    return *this;
  }

  template <class Rep, class Period>
  constexpr void
  setDuration (const std::chrono::duration<Rep, Period> &duration) noexcept
  { m_duration = std::chrono::duration_cast<DurationT> (duration); }

  constexpr DurationT getDuration () const noexcept { return m_duration; }

  void setIntensity (Intensity &&intensity) noexcept
  {
    // use intensity if there is none already or if intensity is an
    // IntensityPair
    if (!m_intensity || intensity.hasPair ())
      {
        m_intensity = std::make_unique<Intensity> (std::move (intensity));
        return;
      }

    // If there is already an intensity, maybe the intensity has already a
    // Level::Low intensity and we want to set Level::High intensity now, so we
    // cannot just overwrite it
    m_intensity->setTarget (intensity.getTarget (intensity.getLevel ()),
                            intensity.getType (), intensity.getLevel ());
  }

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
        m_repeat = repeats;
      }
  }
  int getRepeats () const { return m_repeat; }
  void addRepeat (Repeat &repeats) { m_repeats.emplace_back (repeats); }

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
      // if (m_index >= std::ssize (m_subIntervals)
      if (m_currentInterval == m_subIntervals.end ()
          && m_repeats >= m_parent.m_repeat)
        {
          throw std::out_of_range ("Iterator out of range.");
        }
      // if (m_index == PARENT_INDEX)
      if (returnParent)
        {
          return m_parent;
        }
      return *m_currentInterval;
      // return m_subIntervals[m_index];
    }
    // Throws std::out_of_range
    Interval &operator* () { return getInterval (); }
    // Throws std::out_of_range
    Interval *operator->() { return &getInterval (); }

    IntervalIterator &operator++ () noexcept
    {
      //++m_index;
      if (returnParent)
        {
          returnParent = false;
        }
      else
        {
          ++m_currentInterval;
        }
      // if (m_index >= std::ssize (m_subIntervals))
      if (m_currentInterval == m_subIntervals.end ())
        {
          ++m_repeats;
          if (m_repeats <= m_parent.m_repeat)
            {
              // m_index = PARENT_INDEX;
              returnParent = true;
              m_currentInterval = m_subIntervals.begin ();
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

    // Sentinel is needed here although not used inside of the function
    // NOLINTNEXTLINE
    bool operator== (const Sentinel sentinel) const noexcept
    {
      // return m_index >= std::ssize (m_subIntervals)
      return m_currentInterval == m_subIntervals.end ()
             && m_repeats > m_parent.m_repeat;
    }

  private:
    // NOLINTNEXTLINE
    Interval &m_parent;
    std::span<Interval> m_subIntervals;
    static constexpr int PARENT_INDEX{ -1 };
    // long m_index{ PARENT_INDEX };
    bool returnParent{ true };
    std::span<Interval>::iterator m_currentInterval{ m_subIntervals.begin () };
    int m_repeats{ 1 };
  };

  auto begin ()
  {
    if (m_repeats.size () == 0)
      {
        Repeat all{ .begin = m_intervals.begin (),
                    .last = std::prev (m_intervals.end ()),
                    .times = 1 };
        m_repeats.emplace_back (all);
      }
    return IntervalIterator (*this, m_intervals);
  }
  static auto end () { return Sentinel{}; }
  auto count () const { return m_intervals.size (); }
  auto subIntervalAt (std::size_t index) { return m_intervals.at (index); }

  std::span<Interval> getSubIntervals () { return m_intervals; }

private:
  DurationT m_duration{};
  IntensityT m_intensity;

  Intervals m_intervals;
  Repeats m_repeats;
  int m_repeat{ 1 };
};

} // namespace Workouts