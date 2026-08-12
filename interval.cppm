export module interval;

import config;
import common;
import intensity;
import std;

namespace Workouts
{
export class Interval;
export using intervalReturn = std::expected<Interval, std::string>;
export using Intervals = std::vector<std::shared_ptr<Interval>>;
export using SubIntervals = std::vector<std::weak_ptr<Interval>>;

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
  { m_duration = std::chrono::duration_cast<DurationT> (duration); }

  constexpr DurationT getDuration () const noexcept { return m_duration; }

  void setIntensity (Intensity &&intensity) noexcept
  { m_intensity = std::make_unique<Intensity> (std::move (intensity)); }

  Intensity &getIntensity () { return *m_intensity; }
  const Intensity &getIntensity () const { return *m_intensity; }

  void setRepeats (int repeats) { m_repeats = repeats; }
  int getRepeats () const { return m_repeats; }

  void addSubInterval (Interval &&interval) {}

  voidReturn removeSubInterval (std::size_t index) noexcept {}

  auto begin () const { return m_subIntervals.begin (); }
  auto end () const { return m_subIntervals.end (); }

private:
  DurationT m_duration{};
  IntensityT m_intensity;

  Intervals m_intervals;
  int m_repeats{ 1 };
};

} // namespace Workouts