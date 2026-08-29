export module fitfiles;

import config;
import common;
import filehandling;
import std;
import std.compat;
import fitmodule;
import locale;
import interval;

namespace Workouts
{
namespace fitFiles
{

const constexpr uint16_t AbsolutePowerOffset = 1000;
const constexpr uint8_t AbsoluteHeartRateOffset = 100;
// convert from minutes::seconds to msec.
export constexpr const auto msecInSec{ 1000U };
constexpr const auto secInMinute{ 60U };

/**
 * @brief Set the Locale object
 *
 * Internal function used by \ref sv2wstring and \ref wstring2string
 * @throws std::runtime_error If the system locale cannot be set.
 */
void setLocale ()
{
  if (std::setlocale (LC_ALL, "en_US.UTF-8") == nullptr)
    {
      if (std::setlocale (LC_ALL, "C.UTF-8") == nullptr)
        {
          throw std::runtime_error ("Could not set UTF-8 locale.");
        }
    }
}

/**
 * @brief Converts a UTF-8 std::string_view to a std::wstring.
 *
 * This function relies on the system's locale settings for character
 * conversion using std::mbstowcs.
 *
 * \note This implementation avoids deprecated C++17 features and external
 * dependencies like Boost or ICU.
 *
 * @param utf8String The input string in UTF-8 format.
 * @return std::wstring The converted wide string.
 * @throws std::runtime_error If the conversion from UTF-8 to wide string
 * fails.
 */
export std::wstring sv2wstring (std::string_view utf8String)
{
  setLocale ();
  std::wstring wide_string (utf8String.size (), L'\0');

  auto len{ std::mbstowcs (wide_string.data (), utf8String.data (),
                           utf8String.size ()) };
  if (len == static_cast<std::size_t> (-1))
    {
      throw std::runtime_error (
          std::format ("Cannot convert {} to std::wstring.", utf8String));
    }
  return wide_string;
}

/**
 * @brief Converts std::wstring to utf-8 std::string.
 *
 * Similar to \ref sv2wstring, this function relies on std::wcstombs to avoid
 * deprecated C++17 features or external library dependencies.
 *
 * @param wideString The input wide string to be converted.
 * @return std::string The resulting string in UTF-8 format.
 * @throws std::runtime_error If the conversion from wide string to UTF-8
 * fails.
 */
export std::string wstring2string (std::wstring wideString)
{
  setLocale ();

  // Number of bytes to reserve per character
  constexpr uint8_t bufferMargin{ 4 };

  std::string result (wideString.size () * bufferMargin, '\0');

  auto len{ std::wcstombs (result.data (), wideString.data (),
                           wideString.size () * bufferMargin) };
  if (len == static_cast<std::size_t> (-1))
    {
      throw std::runtime_error ("Cannot convert to utf-8 string.");
    }
  result.resize (len);
  return result;
}

export class FitHandler
{
public:
  explicit FitHandler (const std::filesystem::path &file) : m_file (file)
  { m_inputstream.open (file, std::ios::binary); }

  std::expected<void, std::string> checkFile ()
  {
    if (!m_inputstream.is_open ())
      {
        return std::unexpected (std::format ("Cannot open file {} to read.",
                                             m_file.filename ().string ()));
      }
    return {};
  }

  std::expected<void, std::string> readFile ()
  {
    auto decoder{ std::make_unique<fit::Decode> () };
    try
      {
        // There is no documentation, but apparently this throws if the file is
        // unreadable or invalid.
        if (auto retVal{ decoder->Read (m_inputstream, m_listener) };
            retVal != FIT_TRUE || !m_listener.errMesg.empty ())
          {
            return std::unexpected (m_listener.errMesg);
          }
      }
    catch (std::exception e)
      {
        return std::unexpected (e.what ());
      }
    return {};
  }

  std::string getWorkoutName () { return m_workoutName; }
  std::string getWorkoutNotes () { return m_notes; }

  void setWorkoutName (std::string_view name) { m_workoutName = name; }
  void setWorkoutNotes (std::string_view notes) { m_notes = notes; }

  Intervals &&getIntervals () { return std::move (m_intervals); }

  // public testing Interface
  static constexpr auto getFileHeader () { return getFileID (); }
  intervalReturn getInterval (const fit::WorkoutStepMesg &msg)
  { return m_listener.getFitInterval (msg); }
  void processMesg (fit::Mesg mesg) { m_listener.OnMesg (mesg); }
  void processWktMesg (fit::WorkoutMesg mesg) { m_listener.OnMesg (mesg); }
  void addInterval (Interval &&interval)
  { m_intervals.emplace_back (std::move (interval)); }
  std::string_view getErrMsg () const { return m_listener.errMesg; }

  static constexpr fit::FileIdMesg getFileID ()
  {
    // FileID
    fit::FileIdMesg fileID;
    fileID.SetManufacturer (FIT_MANUFACTURER_DEVELOPMENT);
    fileID.SetType (FIT_FILE_WORKOUT);
    fileID.SetProduct (1);
    fileID.SetTimeCreated (getFitTime ());
    return fileID;
  }

  static fit::WorkoutStepMesg getWorkoutStep (const Interval &interval)
  {
    fit::WorkoutStepMesg msg;
    msg.SetIntensity (FIT_INTENSITY_ACTIVE);
    msg.SetDurationType (FIT_WKT_STEP_DURATION_TIME);
    msg.SetDurationValue (interval.getDuration ().count () * msecInSec);
    auto IntensityUnit{ interval.getIntensity ().getType () };

    std::pair<uintType, uintType> intensity{
      interval.getIntensity ().getTarget (Level::Low),
      interval.getIntensity ().getTarget (Level::High)
    };

    switch (IntensityUnit)
      {
      case IntensityUnit::Watts:
        msg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
        msg.SetCustomTargetPowerLow (intensity.first + AbsolutePowerOffset);
        msg.SetCustomTargetPowerHigh (intensity.second + AbsolutePowerOffset);
        break;
      case IntensityUnit::PercentFTP:
        msg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
        msg.SetCustomTargetPowerLow (intensity.first);
        msg.SetCustomTargetPowerHigh (intensity.second);
        break;
      case IntensityUnit::HeartRateBPM:
        msg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
        msg.SetCustomTargetHeartRateLow (intensity.first
                                         + AbsoluteHeartRateOffset);
        msg.SetCustomTargetHeartRateHigh (intensity.second
                                          + AbsoluteHeartRateOffset);
        break;
      case IntensityUnit::PercentMaxHR:
        msg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
        msg.SetCustomTargetHeartRateLow (intensity.first);
        msg.SetCustomTargetHeartRateHigh (intensity.second);
        break;
      case IntensityUnit::PowerZone:
        msg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
        msg.SetTargetPowerZone (intensity.first);
        break;
      case IntensityUnit::HeartRateZone:
        msg.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
        msg.SetTargetHrZone (intensity.first);
        break;
      default: std::unreachable ();
      }
    return msg;
  }

  static auto getStepMsgs (std::span<Interval> intervals)
  {
    // Calculate the number of intervals with all subIntervals
    // and get WorkoutStepMsg

    int steps{};
    std::vector<fit::Mesg> stepMsgs;
    for (auto &interval : intervals)
      {
        std::chrono::seconds workoutDuration;

        // If there is a subInterval, save the current ID to get
        // where to repeat from
        int from{ steps };
        bool hasSubInterval{ false };

        auto stepMsg{ getWorkoutStep (interval) };
        stepMsg.SetMessageIndex (steps++);
        stepMsgs.push_back (stepMsg);
        workoutDuration += interval.getDuration ();

        std::chrono::seconds subIntervalDuration{ interval.getDuration () };
        for (auto &subInterval : interval.getSubIntervals ())
          {
            hasSubInterval = true;
            auto subStepMsg{ getWorkoutStep (subInterval) };
            subStepMsg.SetMessageIndex (steps++);
            stepMsgs.push_back (subStepMsg);
            subIntervalDuration += subInterval.getDuration ();
          }
        if (hasSubInterval)
          {
            // add repeat Msg
            fit::WorkoutStepMesg repeatMsg;
            repeatMsg.SetDurationType (
                FIT_WKT_STEP_DURATION_REPEAT_UNTIL_STEPS_CMPLT);

            // From where to repeat
            repeatMsg.SetDurationValue (from);

            // How much repeats
            const auto repeats{ interval.getRepeats () };
            repeatMsg.SetTargetValue (repeats);

            // Add the duration of the subInterval loop
            workoutDuration += repeats * subIntervalDuration;

            // repeat Msg also counts as a step
            repeatMsg.SetMessageIndex (steps++);
            stepMsgs.push_back (repeatMsg);
          }
      }
    return stepMsgs;
  }

  static fit::WorkoutMesg getWorkoutMsg (std::string_view workoutName,
                                         std::string_view notes)
  {
    fit::WorkoutMesg workout;
    workout.SetSport (FIT_SPORT_CYCLING);
    // workout.SetSubSport (FIT_SUB_SPORT_INVALID);
    workout.SetWktName (sv2wstring (workoutName));
    workout.SetWktDescription (sv2wstring (notes));
    return workout;
  }

  static voidReturn writeFile (const std::filesystem::path &file,
                               std::string_view workoutName,
                               std::string_view notes,
                               std::span<Interval> intervals)
  {
    // The filestream has to be opened also with std::ios::in to calculate the
    // CRC value
    std::fstream filestream (file, std::ios::in | std::ios::out
                                       | std::ios::binary | std::ios::trunc);
    if (!filestream)
      {
        return std::unexpected (std::format ("Cannot create file {}.",
                                             file.filename ().string ()));
      }
    auto encoder{ std::make_unique<fit::Encode> (fit::ProtocolVersion::V20) };
    if (!encoder)
      {
        return std::unexpected ("Cannot create a fit::Encoder.");
      }
    encoder->Open (filestream);

    // Header i.e. fileIDMesg
    auto fileID{ getFileID () };
    encoder->Write (fileID);

    // WorkoutMesg for Workout
    auto workoutMsg{ getWorkoutMsg (workoutName, notes) };

    // WorkoutStepMesg for Intervals
    auto stepMsgs{ getStepMsgs (intervals) };
    workoutMsg.SetNumValidSteps (stepMsgs.size ());
    encoder->Write (workoutMsg);
    encoder->Write (stepMsgs);

    // Write file;
    if (auto retVal{ encoder->Close () }; retVal != FIT_TRUE)
      {
        return std::unexpected (std::format (
            "Error writing encoded fit messages to file. retVal: {}", retVal));
      }
    return {};
  }

  static long getFitTime ()
  {
    std::tm tm = {};
    std::istringstream FITDate ("1989-12-31 00:00:00");
    FITDate >> std::get_time (&tm, "%Y-%m-%d %H:%M:%S");

    // Convert tm to time_point
    auto tp = std::chrono::system_clock::from_time_t (std::mktime (&tm));

    // Get current time
    auto now = std::chrono::system_clock::now ();

    // Calculate difference in seconds
    auto duration
        = std::chrono::duration_cast<std::chrono::seconds> (now - tp);
    return duration.count ();
  }

  struct Listener : public fit::MesgListener
  {
    Listener (FitHandler *outer) : m_outer (outer) {}

    void OnMesg (fit::Mesg &mesg) override
    {
      if (mesg.GetName () == "file_id")
        {
          fit::FileIdMesg fileId (mesg);
          OnMesg (fileId);
        }
      else if (mesg.GetName () == "workout")
        {
          fit::WorkoutMesg workoutMesg (mesg);
          OnMesg (workoutMesg);
        }
      else if (mesg.GetName () == "workout_step")
        {
          fit::WorkoutStepMesg workoutStepMesg (mesg);
          OnMesg (workoutStepMesg);
        }
    }

    void OnMesg (fit::FileIdMesg &mesg)
    {
      if (mesg.GetType () != FIT_FILE_WORKOUT)
        {
          errMesg = "Not a Workout file.";
        }
    }

    void OnMesg (fit::WorkoutMesg &workoutMesg)
    {
      if (workoutMesg.IsWktNameValid () == FIT_TRUE)
        {
          auto w_workoutName = workoutMesg.GetWktName ();
          m_outer->m_workoutName = wstring2string (w_workoutName);
        }

      if (workoutMesg.IsWktDescriptionValid () == FIT_TRUE)
        {
          auto w_description{ workoutMesg.GetWktDescription () };
          m_outer->m_notes = wstring2string (w_description);
        }
    }

    void OnMesg (fit::WorkoutStepMesg &workoutStepMsg)
    {
      if (auto retVal{ getFitInterval (workoutStepMsg) }; retVal)
        {
          m_outer->addInterval (std::move (*retVal));
        }
      else
        {
          // Account for "abused" error path in case workoutStepMsg
          // contains repeat interval information, which doesn't require
          // adding a new interval
          if (retVal.error () != "Workout repeat step.")
            {
              errMesg = retVal.error ();
            }
        }
    }

    // Don't need getter/setter for a module internal errMsg value
    // NOLINTNEXTLINE
    std::string errMesg;

    intervalReturn getFitInterval (const fit::WorkoutStepMesg &msg)
    {
      Interval interval{};
      if (msg.IsDurationTypeValid () == FIT_TRUE
          && msg.GetDurationType ()
                 == FIT_WKT_STEP_DURATION_REPEAT_UNTIL_STEPS_CMPLT)
        {
          // Get the first interval to repeat from:
          // - Start at the first interval and loop until Interval.index ==
          // msg.duration_value(), which is the index from which to repeat from
          auto repeat_from{ static_cast<std::ptrdiff_t> (
              msg.GetDurationValue ()) };

          // Currently the m_intervals vector holds the parent interval at
          // position 0. All following intervals will be subIntervals of this
          // parent interval. Thus the number of subIntervals is
          // ssize(intervals) - 1. A legal index is always below that number.
          if (repeat_from >= std::ssize (m_outer->m_intervals))
            {
              return std::unexpected (std::format (
                  "Invalid repeat message. No interval at index {}",
                  repeat_from));
            }

          std::vector<Interval>::iterator parentInterval;
          for (uint32_t intervalID{ 0 };
               intervalID < m_outer->m_intervals.size (); ++intervalID)
            {
              if (intervalID == repeat_from)
                {
                  // - Move the following intervals to be Subintervals of this
                  // interval.
                  parentInterval = m_outer->m_intervals.begin () + intervalID;

                  // start with the first subInterval
                  auto it = m_outer->m_intervals.begin () + intervalID + 1;
                  while (it != m_outer->m_intervals.end ())
                    {
                      parentInterval->addSubInterval (std::move (*it));
                      it = m_outer->m_intervals.erase (it);
                    }
                  break;
                }
            }
          // - Get the number of repeats and set Interval.repeat
          if (msg.IsTargetValueValid () != FIT_TRUE)
            {
              return std::unexpected (
                  "No valid Target Value for Interval repeat.");
            }
          auto repeats{ static_cast<uint16_t> (msg.GetTargetValue ()) };

          // Parent interval starts at -1, so every index has to be subtracted
          // by 1
          parentInterval->addRepeat (Repeat{
              .begin = repeat_from - 1,
              .end
              = static_cast<std::ptrdiff_t> (m_outer->m_intervals.size () - 1),
              .times = repeats });

          // This is not very elegant from a design perspective, but returning
          // an empty interval would cause trouble. An alternative would be to
          // use a boolean flag like hasInterval, not much better.
          // Is there a better solution?
          return std::unexpected ("Workout repeat step.");
        }
      if (msg.IsCustomTargetPowerLowValid () == FIT_TRUE)
        {
          auto intensityLow{ static_cast<uint16_t> (
              msg.GetCustomTargetPowerLow ()) };
          applyIntensity (interval, intensityLow, IntensityUnit::Watts,
                          Level::Low);
        }
      if (msg.IsCustomTargetPowerHighValid () == FIT_TRUE)
        {
          auto intensityHigh{ static_cast<uint16_t> (
              msg.GetCustomTargetPowerHigh ()) };
          applyIntensity (interval, intensityHigh, IntensityUnit::Watts,
                          Level::High);
        }
      if (msg.IsCustomTargetHeartRateLowValid () == FIT_TRUE)
        {
          // Don't cast to uint8_t yet because fit heartRates can have values
          // of up to 300
          auto heartRateLow{ msg.GetCustomTargetHeartRateLow () };
          applyIntensity (interval, heartRateLow, IntensityUnit::HeartRateBPM,
                          Level::Low);
        }
      if (msg.IsCustomTargetHeartRateHighValid () == FIT_TRUE)
        {
          auto heartRateHigh{ msg.GetCustomTargetHeartRateHigh () };
          applyIntensity (interval, heartRateHigh, IntensityUnit::HeartRateBPM,
                          Level::High);
        }
      if (msg.IsTargetPowerZoneValid () == FIT_TRUE)
        {
          if (auto pwrZone{ static_cast<uint8_t> (msg.GetTargetPowerZone ()) };
              pwrZone > 0)
            {
              applyIntensity (interval, pwrZone, IntensityUnit::PowerZone);
            }
        }
      if (msg.IsTargetHrZoneValid () == FIT_TRUE)
        {
          auto hrZone{ static_cast<uint8_t> (msg.GetTargetHrZone ()) };
          applyIntensity (interval, hrZone, IntensityUnit::HeartRateZone);
        }
      if (msg.IsDurationTimeValid () == FIT_TRUE)
        {
          auto duration{ msg.GetDurationValue () };
          interval.setDuration (std::chrono::seconds (duration / msecInSec));
          return std::move (interval);
        }
      return std::unexpected ("No Interval data found.");
    }
    // No, this method cannot be static because
    // ftp/maxHeartRate are needed
    // NOLINTNEXTLINE
    constexpr void applyIntensity (Interval &interval, uint16_t intensity,
                                   IntensityUnit unit,
                                   Level level = Level::Low)
    {
      if (unit == IntensityUnit::Watts)
        {
          if (intensity >= AbsolutePowerOffset)
            {
              intensity -= AbsolutePowerOffset;
              interval.setIntensity (Intensity{
                  intensity, IntensityUnit::Watts, m_outer->m_ftp, level });
            }
          else
            {
              interval.setIntensity (Intensity{ intensity,
                                                IntensityUnit::PercentFTP,
                                                m_outer->m_ftp, level });
            }
        }
      else if (unit == IntensityUnit::PowerZone)
        {
          interval.setIntensity (Intensity{
              intensity, IntensityUnit::PowerZone, m_outer->m_ftp, level });
        }
      else if (unit == IntensityUnit::HeartRateBPM)
        {
          if (intensity >= AbsoluteHeartRateOffset)
            {
              intensity -= AbsoluteHeartRateOffset;
              interval.setIntensity (
                  Intensity{ intensity, IntensityUnit::HeartRateBPM,
                             m_outer->m_maxHeartRate, level });
            }
          else
            {
              interval.setIntensity (
                  Intensity{ intensity, IntensityUnit::PercentMaxHR,
                             m_outer->m_maxHeartRate, level });
            }
        }
      else if (unit == IntensityUnit::HeartRateZone)
        {
          interval.setIntensity (Intensity{ intensity,
                                            IntensityUnit::HeartRateZone,
                                            m_outer->m_maxHeartRate, level });
        }
    }

  private:
    // Non-owning, so raw ptr is ok here and it shouldn't be null because it is
    // sensibly initialized in the constructor
    FitHandler *m_outer;
  } m_listener{ this };

private:
  std::filesystem::path m_file;
  std::ifstream m_inputstream;
  static constexpr uint8_t FIT_SUCCEED{ 0U };
  static constexpr uint8_t FIT_TRUE{ 1U };
  std::string m_workoutName;
  std::string m_notes;
  std::vector<Interval> m_intervals;
  uint16_t m_ftp{};
  uint8_t m_maxHeartRate{};
};
}; // namespace fitFiles
}; // namespace Workouts