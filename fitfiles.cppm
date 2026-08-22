export module fitfiles;

import config;
import common;
import filehandling;
import std;
import std.compat;
import fitmodule;
import interval;

namespace Workouts
{
namespace fitFiles
{

const constexpr uint16_t AbsolutePowerOffset = 1000;
const constexpr uint8_t AbsoluteHeartRateOffset = 100;
// convert from minutes::seconds to msec.
constexpr const auto msecInSec{ 1000U };
constexpr const auto secInMinute{ 60U };

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
    if (decoder->Read (m_inputstream, m_listener) != FIT_TRUE)
      {
        return std::unexpected (m_listener.errMesg);
      }
    return {};
  }

  std::string getWorkoutName () { return m_workoutName; }
  std::string getWorkoutNotes () { return m_notes; }

  void setWorkoutName (std::string_view name) { m_workoutName = name; }
  void setWorkoutNotes (std::string_view notes) { m_notes = notes; }

  Intervals &&getIntervals () { return std::move (m_intervals); }

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

  fit::WorkoutMesg getWorkoutMsg ()
  {
    fit::WorkoutMesg workout;
    workout.SetSport (FIT_SPORT_CYCLING);
    // workout.SetSubSport (FIT_SUB_SPORT_INVALID);
    workout.SetWktName (
        std::wstring (m_workoutName.begin (), m_workoutName.end ()));
    workout.SetWktDescription (
        std::wstring (m_notes.begin (), m_notes.end ()));
    return workout;
  }

  voidReturn writeFile (std::span<Interval> intervals)
  {
    // The filestream has to be opened also with std::ios::in to calculate the
    // CRC value
    std::fstream filestream (m_file, std::ios::in | std::ios::out
                                         | std::ios::binary | std::ios::trunc);
    if (!filestream)
      {
        return std::unexpected (std::format ("Cannot create file {}.",
                                             m_file.filename ().string ()));
      }
    auto encoder{ std::make_unique<fit::Encode> (fit::ProtocolVersion::V20) };
    if (!encoder)
      {
        return std::unexpected ("Cannot create a fit::Encoder.");
      }
    encoder->Open (filestream);

    // FileID
    fit::FileIdMesg fileID;
    fileID.SetManufacturer (FIT_MANUFACTURER_DEVELOPMENT);
    fileID.SetType (FIT_FILE_WORKOUT);
    fileID.SetProduct (1);
    fileID.SetTimeCreated (getFitTime ());
    encoder->Write (fileID);

    // WorkoutMesg
    auto workoutMsg{ getWorkoutMsg () };

    // Calculate the number of intervals with all subIntervals
    // and get WorkoutStepMsg
    int steps{};
    std::vector<fit::Mesg> stepMsgs;
    std::chrono::seconds workoutDuration;

    for (auto &interval : intervals)
      {
        // If there is a subInterval, save the current ID to get where to
        // repeat from
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
    workoutMsg.SetNumValidSteps (steps);
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

private:
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
      auto mesgName = mesg.GetName ();
      if (mesgName == "file_id")
        {
          fit::FileIdMesg fileIdMesg (mesg);
        }
      else if (mesgName == "workout")
        {
          fit::WorkoutMesg workoutMesg (mesg);
          if (workoutMesg.IsWktNameValid () != FIT_SUCCEED)
            {
              auto w_workoutName = workoutMesg.GetWktName ();
              m_outer->m_workoutName = std::string{ w_workoutName.begin (),
                                                    w_workoutName.end () };
            }
          if (workoutMesg.IsWktDescriptionValid () != FIT_SUCCEED)
            {
              auto w_description{ workoutMesg.GetWktDescription () };
              m_outer->m_notes = std::string{ w_description.begin (),
                                              w_description.end () };
            }
        }
      else if (mesgName == "workout_step")
        {
          fit::WorkoutStepMesg workoutStepMsg (mesg);
          if (auto retVal{ getFitInterval (workoutStepMsg) }; retVal)
            {
              m_outer->m_intervals.emplace_back (*retVal);
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
    }
    // Don't need getter/setter for a module internal errMsg value
    // NOLINTNEXTLINE
    std::string errMesg;

  private:
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
          uint32_t repeat_from{ msg.GetDurationValue () };
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
          parentInterval->setRepeats (repeats);
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
          auto heartRateLow{ static_cast<uint8_t> (
              msg.GetCustomTargetHeartRateLow ()) };
          applyIntensity (interval, heartRateLow, IntensityUnit::HeartRateBPM,
                          Level::Low);
        }
      if (msg.IsCustomTargetHeartRateHighValid () == FIT_TRUE)
        {
          auto heartRateHigh{ static_cast<uint8_t> (
              msg.GetCustomTargetHeartRateHigh ()) };
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
          interval.setDuration (std::chrono::seconds (duration));
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
          interval.setIntensity (Intensity{
              intensity, IntensityUnit::PercentFTP, m_outer->m_ftp, level });
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
          interval.setIntensity (Intensity{ intensity,
                                            IntensityUnit::PercentMaxHR,
                                            m_outer->m_maxHeartRate, level });
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