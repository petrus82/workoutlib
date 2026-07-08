export module interval;

import config;
import common;
import intensity;
import std;

namespace Workouts
{
export class Interval;
export using intervalReturn = std::expected<Interval, std::string>;
export using Intervals = std::vector<std::unique_ptr<Interval>>;
export using IntervalView = std::vector<std::reference_wrapper<Interval>>;
export using IteratorType = Intervals::iterator;
export using IteratorViewType = IntervalView::iterator;

/**
 * @brief Holds intensity, duration and a vector of sub-intervals.
 *
 * To construct an Interval, create an instance of PowerAbsolute,
 * PowerRelative, PowerZone, HeartRateAbsolute, HeartRateRelative or
 * HeartRateZone using their create functions. Create a default Interval
 * instance, pass interval and instance to the static set function of Interval.
 *
 * This approach is chosen because it enables the creation of an empty Interval
 * class in case intensity and / or duration are not known at construction of
 * Interval.
 *
 * Additionally this ensures validity of the intensity data and an
 * expressive error message without using exceptions by using std::expected.
 */
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
      : m_duration{ copy.m_duration }, m_repeats{ copy.m_repeats },
        m_subIntervals{ copy.m_subIntervals.begin (),
                        copy.m_subIntervals.end () }
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
    m_subIntervals
        = { copy.m_subIntervals.begin (), copy.m_subIntervals.end () };
    Intensity intensityCopy (*copy.m_intensity);
    m_intensity = std::make_unique<Intensity> (std::move (intensityCopy));
    return *this;
  }
  Interval (Interval &&other) noexcept
      : m_duration (other.m_duration), m_repeats (other.m_repeats),
        m_subIntervals (std::move (other.m_subIntervals)),
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
    m_subIntervals = std::move (other.m_subIntervals);
    m_intensity = std::move (other.m_intensity);
    return *this;
  }

  template <class Rep, class Period>
  constexpr void
  setDuration (const std::chrono::duration<Rep, Period> &duration) noexcept
  { m_duration = std::chrono::duration_cast<std::chrono::seconds> (duration); }

  constexpr std::chrono::seconds getDuration () const noexcept
  { return m_duration; }

  void setIntensity (Intensity &&intensity) noexcept
  { m_intensity = std::make_unique<Intensity> (std::move (intensity)); }

  Intensity &getIntensity () { return *m_intensity; }
  const Intensity &getIntensity () const { return *m_intensity; }

  void setRepeats (int repeats) { m_repeats = repeats; }

  void addSubInterval (Interval &&interval)
  { m_subIntervals.emplace_back (std::move (interval)); }

  voidReturn removeSubInterval (std::size_t index) noexcept
  {
    auto it{ m_subIntervals.begin () };
    std::advance (it, index);
    if (it != m_subIntervals.end ())
      {
        m_subIntervals.erase (it);
        return {};
      }
    return std::unexpected (
        std::format ("Interval of index {} does not exist.", index));
  }

  /**
   * @brief Provides an iterable view over Interval, while expanding the
   * repeats.
   * It manages two indices: pos_in_block to cycle through
   * sub-intervals and repeat_index to account for the number of repeats.
   *
   * expandedView implements std::ranges::view_interface, thus enabling
   * range-based iterations using begin() and end().
   *
   */
  struct expandedView : std::ranges::view_interface<expandedView>
  {
    const Interval *self = nullptr;

    struct iterator
    {
      const Interval *self = nullptr;
      std::size_t repeat_index{ 0 };
      std::size_t pos_in_block{ 0 };

      using value_type = const Interval &;
      using difference_type = std::ptrdiff_t;

      const Interval &operator* () const
      {
        if (pos_in_block == 0)
          {
            return *self;
          }
        return self->m_subIntervals[pos_in_block - 1];
      }

      iterator &operator++ ()
      {
        ++pos_in_block;
        if (pos_in_block > self->m_subIntervals.size ())
          {
            pos_in_block = 0;
            ++repeat_index;
          }
        return *this;
      }

      void operator++ (int) { ++(*this); }

      friend bool operator== (const iterator &it, std::default_sentinel_t)
      {
        return it.repeat_index
               >= static_cast<std::size_t> (it.self->m_repeats);
      }
    };

    expandedView () = default;
    constexpr expandedView (std::nullptr_t, const Interval *p) : self (p) {}

    iterator begin () const { return { self, 0, 0 }; }
    static std::default_sentinel_t end () { return {}; }
  };

  constexpr auto getIntervalsExpanded () noexcept
  { return expandedView{ nullptr, this }; }

private:
  std::chrono::seconds m_duration{};
  std::vector<Interval> m_subIntervals;
  std::unique_ptr<Intensity> m_intensity = std::make_unique<Intensity> ();
  int m_repeats{ 1 };
};

} // namespace Workouts