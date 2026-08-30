#include <gtest/gtest.h>

import std;
import workoutlib;
import fitmodule;
import fitfiles;
import file_concept;
import sha256;

// Test Workout.fit file generated using xxd -i Workout.fit
constexpr std::array WorkoutFile{
  0x0e, 0x20, 0xa6, 0x52, 0x64, 0x01, 0x00, 0x00, 0x2e, 0x46, 0x49, 0x54, 0x88,
  0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x02, 0x84, 0x00, 0x01, 0x00,
  0x02, 0x02, 0x84, 0x04, 0x04, 0x86, 0x00, 0xff, 0x00, 0x05, 0x01, 0x00, 0xd1,
  0xb0, 0xf4, 0x44, 0x40, 0x00, 0x00, 0x1a, 0x00, 0x04, 0x04, 0x01, 0x00, 0x08,
  0x0c, 0x07, 0x11, 0x42, 0x07, 0x06, 0x02, 0x84, 0x00, 0x02, 0x48, 0x49, 0x54,
  0x20, 0x57, 0x6f, 0x72, 0x6b, 0x6f, 0x75, 0x74, 0x00, 0x48, 0x49, 0x54, 0x20,
  0x49, 0x6e, 0x74, 0x65, 0x72, 0x76, 0x61, 0x6c, 0x20, 0x6d, 0x69, 0x74, 0x20,
  0x34, 0x20, 0x6d, 0x69, 0x6e, 0x2e, 0x20, 0x56, 0x4f, 0x32, 0x4d, 0x61, 0x78,
  0x2c, 0x20, 0x31, 0x32, 0x78, 0x33, 0x30, 0x2f, 0x33, 0x30, 0x20, 0x75, 0x6e,
  0x64, 0x20, 0x53, 0x77, 0x65, 0x65, 0x74, 0x20, 0x53, 0x70, 0x6f, 0x74, 0x20,
  0x49, 0x6e, 0x74, 0x65, 0x72, 0x76, 0x61, 0x6c, 0x2e, 0x00, 0x09, 0x00, 0x40,
  0x00, 0x00, 0x1b, 0x00, 0x07, 0x07, 0x01, 0x00, 0x01, 0x01, 0x00, 0x02, 0x04,
  0x86, 0x03, 0x01, 0x00, 0x05, 0x04, 0x86, 0x06, 0x04, 0x86, 0xfe, 0x02, 0x84,
  0x00, 0x00, 0x00, 0xc0, 0x27, 0x09, 0x00, 0x04, 0x32, 0x00, 0x00, 0x00, 0x3c,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xa9, 0x03, 0x00, 0x04,
  0x69, 0x00, 0x00, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
  0xe0, 0x93, 0x04, 0x00, 0x04, 0x32, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x00, 0x00, 0x30, 0x75, 0x00, 0x00, 0x04, 0x73, 0x00, 0x00,
  0x00, 0x82, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x30, 0x75, 0x00,
  0x00, 0x04, 0x32, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x04, 0x00, 0x40,
  0x00, 0x00, 0x1b, 0x00, 0x04, 0x01, 0x01, 0x00, 0x02, 0x04, 0x86, 0x04, 0x04,
  0x86, 0xfe, 0x02, 0x84, 0x00, 0x06, 0x03, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00,
  0x00, 0x05, 0x00, 0x40, 0x00, 0x00, 0x1b, 0x00, 0x07, 0x07, 0x01, 0x00, 0x01,
  0x01, 0x00, 0x02, 0x04, 0x86, 0x03, 0x01, 0x00, 0x05, 0x04, 0x86, 0x06, 0x04,
  0x86, 0xfe, 0x02, 0x84, 0x00, 0x00, 0x00, 0xc0, 0x27, 0x09, 0x00, 0x04, 0x32,
  0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0xc0,
  0x27, 0x09, 0x00, 0x04, 0x55, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0x00, 0x07,
  0x00, 0x00, 0x00, 0x00, 0xe0, 0x93, 0x04, 0x00, 0x04, 0x32, 0x00, 0x00, 0x00,
  0x3c, 0x00, 0x00, 0x00, 0x08, 0x00, 0xca, 0x6f
};
// Minimal Activity fit file
static constexpr std::array ActivityContent{
  0x0e, 0x20, 0x48, 0x08, 0x8c, 0x00, 0x00, 0x00, 0x2e, 0x46, 0x49, 0x54, 0x70,
  0x4e, 0x40, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x00, 0x01, 0x02, 0x04,
  0x04, 0x04, 0x06, 0x00, 0x04, 0xff, 0x00, 0x00, 0xb6, 0xf5, 0x44, 0x41, 0x00,
  0x00, 0x12, 0x00, 0x04, 0xfd, 0x04, 0x06, 0x02, 0x04, 0x06, 0x07, 0x04, 0x06,
  0x08, 0x04, 0x06, 0x01, 0x00, 0xb6, 0xf5, 0x44, 0x00, 0xb6, 0xf5, 0x44, 0xe8,
  0x03, 0x00, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x42, 0x00, 0x00, 0x13, 0x00, 0x04,
  0xfd, 0x04, 0x06, 0x02, 0x04, 0x06, 0x07, 0x04, 0x06, 0x08, 0x04, 0x06, 0x02,
  0x00, 0xb6, 0xf5, 0x44, 0x00, 0xb6, 0xf5, 0x44, 0xe8, 0x03, 0x00, 0x00, 0xe8,
  0x03, 0x00, 0x00, 0x43, 0x00, 0x00, 0x14, 0x00, 0x02, 0xfd, 0x04, 0x06, 0x05,
  0x04, 0x06, 0x03, 0x00, 0xb6, 0xf5, 0x44, 0x00, 0x00, 0x00, 0x00, 0x44, 0x00,
  0x00, 0x22, 0x00, 0x03, 0xfd, 0x04, 0x06, 0x05, 0x04, 0x06, 0x02, 0x02, 0x04,
  0x04, 0x00, 0xb6, 0xf5, 0x44, 0x00, 0xb6, 0xf5, 0x44, 0x01, 0x00, 0xea, 0x43
};

namespace Workouts
{

template <FileHandlerC HandlerType> class DataTestContainer
{
public:
  DataTestContainer () = default;
  virtual ~DataTestContainer () = default;
  DataTestContainer (const DataTestContainer &other) = default;
  DataTestContainer &operator= (const DataTestContainer &other) = default;
  DataTestContainer (DataTestContainer &&other) = default;
  DataTestContainer &operator= (DataTestContainer &&other) = default;

  virtual void setUp () = 0;
  virtual void setUpIntervals () = 0;
  virtual void cleanUp () = 0;
  virtual HandlerType &invalidTestFile () = 0;
  virtual HandlerType &wrongFileContent () = 0;
  virtual std::string testWorkoutName () = 0;
  virtual std::string testWorkoutNotes () = 0;
  virtual intervalReturn testAbsolutePower () = 0;
  virtual intervalReturn testRelativePower () = 0;
  virtual intervalReturn testPowerZone () = 0;
  virtual intervalReturn testHrBPM () = 0;
  virtual intervalReturn testHrPercentMax () = 0;
  virtual intervalReturn testHrZone () = 0;
  virtual std::expected<Intervals, std::string> testSubIntervals () = 0;
  virtual intervalReturn testRepeatMessage () = 0;
  virtual stringReturn testInvalidRepeatMessage () = 0;
  virtual voidReturn generateReferenceFile () = 0;
  virtual std::filesystem::path getReferenceFile () const = 0;
  virtual std::string_view getHash () const = 0;
  virtual stringReturn getFileContent () = 0;
  virtual std::span<std::string> getTestTokens () = 0;

  static constexpr std::string_view workoutName () { return WorkoutName; }
  static constexpr std::string_view workoutNotes () { return WorkoutNotes; }

  static constexpr uint16_t absolutePowerLo () { return AbsPowerLo; }
  static constexpr uint16_t absolutePowerHi () { return AbsPowerHi; }
  static constexpr std::string_view absoluteUnitString ()
  { return AbsPowerUnitString; }
  static constexpr uint16_t relPowerLo () { return RelPowerLo; }
  static constexpr uint16_t relPowerHi () { return RelPowerHi; }
  static constexpr std::string_view relPowerUnitStr ()
  { return RelPowerUnitStr; }
  static constexpr uint16_t powerZone () { return PowerZone; }
  static constexpr std::string_view powerZoneUnitStr ()
  { return PowerZoneUnitStr; }

  static constexpr uint16_t absoluteHrLo () { return AbsHrLo; }
  static constexpr uint16_t absoluteHrHi () { return AbsHrHi; }
  static constexpr std::string_view absoluteHrUnitString ()
  { return AbsHrUnitString; }
  static constexpr uint16_t ftp () { return Ftp; }
  static constexpr uint16_t relHrLo () { return RelHrLo; }
  static constexpr uint16_t relHrHi () { return RelHrHi; }
  static constexpr uint16_t maxHr () { return MaxHr; }
  static constexpr std::string_view relHrUnitStr () { return RelHrUnitStr; }
  static constexpr uint16_t hrZone () { return HrZone; }
  static constexpr std::string_view hrZoneUnitStr () { return HrZoneUnitStr; }

  static constexpr uint16_t parentLoInt () { return ParentLoInt; }
  static constexpr uint16_t parentHiInt () { return ParentHiInt; }
  static constexpr std::chrono::seconds parentDur () { return ParentDur; }

  static constexpr uint16_t subLoInt () { return SubLoInt; }
  static constexpr uint16_t subHiInt () { return SubHiInt; }
  static constexpr std::chrono::seconds subDur () { return SubDur; }
  static constexpr uint8_t subIntervalRepeats () { return SubIntervalRep; }
  static constexpr std::string_view workoutRepeatStr ()
  { return WorkoutRepeatStr; }
  static constexpr std::string_view illegalMessageRepeatStr ()
  { return IllegalMessageRepStr; }

private:
  static constexpr std::string_view WorkoutName{ "Workout" };
  static constexpr std::string_view WorkoutNotes{
    "This is a longer Note with longer lines which have no meaning"
    "\nbut some linebreaks and a bunch of crazy characters "
    "\nlike these: ÄÖÜäöüß!?.,;:@|<>"
  };

  static const constexpr uint16_t AbsPowerLo{ 100 };
  static const constexpr uint16_t AbsPowerHi{ 200 };
  static constexpr std::string_view AbsPowerUnitString{ "watts" };
  static constexpr uint16_t Ftp{ 300 };
  static const constexpr uint16_t RelPowerLo{ 50 };
  static const constexpr uint16_t RelPowerHi{ 80 };
  static constexpr std::string_view RelPowerUnitStr{ "\%FTP" };
  static const constexpr uint16_t PowerZone{ 4 };
  static constexpr std::string_view PowerZoneUnitStr{ "power zone" };

  static const constexpr uint16_t AbsHrLo{ 120 };
  static const constexpr uint16_t AbsHrHi{ 150 };
  static const constexpr uint16_t MaxHr{ 200 };
  static constexpr std::string_view AbsHrUnitString{ "bpm" };
  static const constexpr uint16_t RelHrLo{ 50 };
  static const constexpr uint16_t RelHrHi{ 80 };
  static constexpr std::string_view RelHrUnitStr{ "\%max heart rate" };
  static const constexpr uint16_t HrZone{ 4 };
  static constexpr std::string_view HrZoneUnitStr{ "heart rate zone" };

  static constexpr const uint16_t ParentLoInt{ 88 };
  static constexpr const uint16_t ParentHiInt{ 93 };
  static constexpr const std::chrono::seconds ParentDur{ 1 };

  static constexpr const uint16_t SubLoInt{ 50 };
  static constexpr const uint16_t SubHiInt{ 65 };
  static constexpr const std::chrono::seconds SubDur{ 2 };
  static constexpr uint8_t SubIntervalRep{ 4 };
  static constexpr std::string_view WorkoutRepeatStr{ "Workout repeat step." };
  static constexpr std::string_view IllegalMessageRepStr{
    "Invalid repeat message. No interval at index 2"
  };
};
namespace fitFiles
{
class FitDataTestContainer : public DataTestContainer<FitHandler>
{
public:
  FitDataTestContainer () {}
  ~FitDataTestContainer () override = default;
  FitDataTestContainer (const FitDataTestContainer &other) = delete;
  FitDataTestContainer &operator= (const FitDataTestContainer &other) = delete;
  FitDataTestContainer (FitDataTestContainer &&other) noexcept = default;
  FitDataTestContainer &operator= (FitDataTestContainer &&other) = default;

  void setUp () override
  {
    // Generate testfiles before the file handler
    m_testfileHandler = std::make_unique<FitHandler> (m_testfile);
    if (!std::filesystem::exists (m_activity))
      {
        // Write ActivityContent to file
        std::ofstream output (m_activity, std::ios::binary);
        if (!output.is_open ())
          {
            throw std::runtime_error ("Cannot write testfile.");
          }
        for (const auto &byte : ActivityContent)
          {
            output.put (static_cast<char> (byte));
          }
      }
    m_activityHandler = std::make_unique<FitHandler> (m_activity);
  }

  void setUpIntervals () override
  {
    m_wktStep.SetMessageIndex (0);
    m_wktStep.SetIntensity (FIT_INTENSITY_ACTIVE);
    m_wktStep.SetDurationType (FIT_WKT_STEP_DURATION_TIME);
    m_wktStep.SetDurationTime (1);
  }
  void cleanUp () override
  {
    for (const auto &file : m_garbage)
      {
        if (std::filesystem::exists (file))
          {
            std::filesystem::remove (file);
          }
      }
  }

  FitHandler &invalidTestFile () override { return m_nonexistentHandler; }
  FitHandler &wrongFileContent () override { return *m_activityHandler; }
  std::string testWorkoutName () override
  {
    fit::WorkoutMesg workoutMsg;
    workoutMsg.SetWktName (sv2wstring (workoutName ()));
    m_testfileHandler->processWktMesg (workoutMsg);
    std::string result{ m_testfileHandler->getWorkoutName () };
    return result;
  }
  std::string testWorkoutNotes () override
  {
    fit::WorkoutMesg workoutMsg;
    workoutMsg.SetWktDescription (sv2wstring (workoutNotes ()));
    fit::Mesg mesg (workoutMsg);
    m_testfileHandler->processMesg (mesg);
    std::string result{ m_testfileHandler->getWorkoutNotes () };
    return result;
  }
  intervalReturn testAbsolutePower () override
  {
    m_wktStep.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
    m_wktStep.SetCustomTargetPowerLow (absolutePowerLo ()
                                       + AbsolutePowerOffset);
    m_wktStep.SetCustomTargetPowerHigh (absolutePowerHi ()
                                        + AbsolutePowerOffset);
    return m_testfileHandler->getInterval (m_wktStep);
  }
  intervalReturn testRelativePower () override
  {
    m_wktStep.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
    m_wktStep.SetCustomTargetPowerLow (relPowerLo ());
    m_wktStep.SetCustomTargetPowerHigh (relPowerHi ());
    return m_testfileHandler->getInterval (m_wktStep);
  }
  intervalReturn testPowerZone () override
  {
    m_wktStep.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
    m_wktStep.SetTargetPowerZone (powerZone ());
    return m_testfileHandler->getInterval (m_wktStep);
  }
  intervalReturn testHrBPM () override
  {
    m_wktStep.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
    m_wktStep.SetCustomTargetHeartRateLow (absoluteHrLo () + AbsoluteHrOffset);
    m_wktStep.SetCustomTargetHeartRateHigh (absoluteHrHi ()
                                            + AbsoluteHrOffset);
    return m_testfileHandler->getInterval (m_wktStep);
  }
  intervalReturn testHrPercentMax () override
  {
    m_wktStep.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
    m_wktStep.SetCustomTargetHeartRateLow (relHrLo ());
    m_wktStep.SetCustomTargetHeartRateHigh (relHrHi ());
    return m_testfileHandler->getInterval (m_wktStep);
  }
  intervalReturn testHrZone () override
  {
    m_wktStep.SetTargetType (FIT_WKT_STEP_TARGET_HEART_RATE);
    m_wktStep.SetTargetHrZone (hrZone ());
    return m_testfileHandler->getInterval (m_wktStep);
  }
  std::expected<Intervals, std::string> testSubIntervals () override
  {
    if (auto setup{ setUpSubIntervals () }; !setup)
      {
        return std::unexpected (setup.error ());
      }

    // legal repeat: from parent (index 0), 2 times
    fit::WorkoutStepMesg repeatMsg = m_wktStep;
    repeatMsg.SetDurationType (FIT_WKT_STEP_DURATION_REPEAT_UNTIL_STEPS_CMPLT);
    repeatMsg.SetTargetValue (2);
    repeatMsg.SetDurationValue (0);
    auto repeat{ m_testfileHandler->getInterval (repeatMsg) };
    if (!repeat && repeat.error () != "Workout repeat step.")
      {
        return std::unexpected (repeat.error ());
      }
    return m_testfileHandler->getIntervals ();
  }
  intervalReturn testRepeatMessage () override
  {
    if (auto setup{ setUpSubIntervals () }; !setup)
      {
        return std::unexpected (setup.error ());
      }

    fit::WorkoutStepMesg repeatMsg = m_wktStep;
    repeatMsg.SetDurationType (FIT_WKT_STEP_DURATION_REPEAT_UNTIL_STEPS_CMPLT);
    repeatMsg.SetTargetValue (2);
    repeatMsg.SetDurationValue (0);
    return m_testfileHandler->getInterval (repeatMsg);
  }
  stringReturn testInvalidRepeatMessage () override
  {
    if (auto setup{ setUpSubIntervals () }; !setup)
      {
        return std::unexpected (setup.error ());
      }

    // illegal index above number of subIntervals
    fit::WorkoutStepMesg repeatMsg = m_wktStep;
    repeatMsg.SetDurationType (FIT_WKT_STEP_DURATION_REPEAT_UNTIL_STEPS_CMPLT);
    repeatMsg.SetTargetValue (2);
    repeatMsg.SetDurationValue (2);
    m_testfileHandler->processMesg (fit::Mesg (repeatMsg));
    return std::string{ m_testfileHandler->getErrMsg () };
  }
  std::string_view getHash () const override { return m_Hash; }
  voidReturn generateReferenceFile () override
  {
    // For every item added to the binary a text item is added to the
    // m_testTokens vector that later has to be found in the resulting
    // .csv file

    // Use an incrementing index for the interval duration to check if a
    // specific interval is included. Thus the interval duration in seconds is
    // the number of the interval added. The FitCSVTool returns a csv with the
    // interval duration in fractions of seconds (e.g. "1.0" seconds).
    uint8_t intervalIndex{};

    std::string testToken{};
    Workout workout{ workoutName (), workoutNotes () };
    m_testTokens.emplace_back (workoutName ());
    m_testTokens.emplace_back (workoutNotes ());

    Interval powerAbs{ Intensity{ IntensityPair{ absolutePowerLo (),
                                                 absolutePowerHi () },
                                  IntensityUnit::Watts, ftp () },
                       std::chrono::seconds (++intervalIndex) };
    testToken = "custom_target_power_low,\"";
    testToken
        .append (std::to_string (absolutePowerLo () + AbsolutePowerOffset))
        .append ("\"");
    m_testTokens.emplace_back (testToken);
    testToken = "custom_target_power_high,\"";
    testToken
        .append (std::to_string (absolutePowerHi () + AbsolutePowerOffset))
        .append ("\"");
    m_testTokens.emplace_back (testToken);
    testToken = "duration_time,\"" + std::to_string (intervalIndex) + ".0\"";
    m_testTokens.emplace_back (testToken);

    powerAbs.addSubInterval (
        Interval{ Intensity{ IntensityPair{ relPowerLo (), relPowerHi () },
                             IntensityUnit::PercentFTP, ftp () },
                  std::chrono::seconds (++intervalIndex) });
    testToken = "custom_target_power_low,\"";
    testToken.append (std::to_string (relPowerLo ())).append ("\"");
    m_testTokens.emplace_back (testToken);
    testToken = "custom_target_power_high,\"";
    testToken.append (std::to_string (relPowerHi ())).append ("\"");
    m_testTokens.emplace_back (testToken);
    testToken = "duration_time,\"" + std::to_string (intervalIndex) + ".0\"";
    m_testTokens.emplace_back (testToken);

    powerAbs.addRepeat (Repeat{ .begin = -1, .end = 0, .times = 1 });
    // Repeat beginning in fit language is .begin +1
    testToken = "duration_step,\"0\"";
    m_testTokens.emplace_back (testToken);
    // Repeat times in fit language
    testToken = "repeat_steps,\"1\"";
    m_testTokens.emplace_back (testToken);

    workout.addInterval (std::move (powerAbs));

    workout.addInterval (
        Interval{ Intensity{ powerZone (), IntensityUnit::PowerZone, ftp () },
                  std::chrono::seconds (++intervalIndex) });
    testToken = "target_power_zone,\"";
    testToken.append (std::to_string (powerZone ())).append ("\"");
    m_testTokens.emplace_back (testToken);
    testToken = "duration_time,\"" + std::to_string (intervalIndex) + ".0\"";
    m_testTokens.emplace_back (testToken);

    workout.addInterval (
        Interval{ Intensity{ IntensityPair{ absoluteHrLo (), absoluteHrHi () },
                             IntensityUnit::HeartRateBPM, maxHr () },
                  std::chrono::seconds (++intervalIndex) });
    testToken = "custom_target_heart_rate_low,\"";
    testToken.append (std::to_string (absoluteHrLo () + AbsoluteHrOffset))
        .append ("\"");
    m_testTokens.emplace_back (testToken);
    testToken = "custom_target_heart_rate_high,\"";
    testToken.append (std::to_string (absoluteHrHi () + AbsoluteHrOffset))
        .append ("\"");
    m_testTokens.emplace_back (testToken);
    testToken = "duration_time,\"" + std::to_string (intervalIndex) + ".0\"";
    m_testTokens.emplace_back (testToken);

    workout.addInterval (
        Interval{ Intensity{ IntensityPair{ relHrLo (), relHrHi () },
                             IntensityUnit::PercentMaxHR, maxHr () },
                  std::chrono::seconds (++intervalIndex) });
    testToken = "custom_target_heart_rate_low,\"";
    testToken.append (std::to_string (relHrLo ())).append ("\"");
    m_testTokens.emplace_back (testToken);
    testToken = "custom_target_heart_rate_high,\"";
    testToken.append (std::to_string (relHrHi ())).append ("\"");
    m_testTokens.emplace_back (testToken);
    testToken = "duration_time,\"" + std::to_string (intervalIndex) + ".0\"";
    m_testTokens.emplace_back (testToken);

    workout.addInterval (Interval{
        Intensity{ hrZone (), IntensityUnit::HeartRateZone, maxHr () },
        std::chrono::seconds (++intervalIndex) });
    testToken = "target_hr_zone,\"";
    testToken.append (std::to_string (hrZone ()));
    testToken.append ("\"");

    m_testTokens.emplace_back (testToken);
    testToken = "duration_time,\"" + std::to_string (intervalIndex) + ".0\"";
    m_testTokens.emplace_back (testToken);

    return workout.writeFile (m_referenceHandler, m_reference);
  }

  stringReturn getFileContent () override
  {
    constexpr std::string_view FitCSV{ "/usr/lib/garminfit/FitCSVTool.jar" };

    std::filesystem::path csvFile{ m_reference };
    csvFile.replace_extension ("csv");

    return
        [FitCSV] ()
            -> voidReturn
                 {
                   if (!std::filesystem::exists (FitCSV))
                     {
                       return std::unexpected (
                           std::format ("FitCSVTool not found in {}", FitCSV));
                     }
                   return {};
                 }()
                     .and_then ([this] { return generateReferenceFile (); })
                     .and_then (
                         [this] () -> voidReturn
                           {
                             if (!std::filesystem::exists (m_reference))
                               {
                                 return std::unexpected (std::format (
                                     "Cannot find {}", m_reference.string ()));
                               }
                             return {};
                           })
                     .and_then (
                         [this, FitCSV, csvFile] () -> voidReturn
                           {
                             m_garbage.emplace_back (csvFile);

                             std::string cmdString{ "java -jar " };
                             cmdString.append (FitCSV);
                             cmdString.append (std::format (
                                 " -b {} {}", m_reference.string (),
                                 csvFile.string ()));
                             if (std::system (cmdString.c_str ()) != 0)
                               {
                                 return std::unexpected (
                                     "Call to FitCSV failed.");
                               }
                             return {};
                           })
                     .and_then (
                         [csvFile] () -> stringReturn
                           {
                             std::ifstream fileContent{ csvFile,
                                                        std::ios::in };
                             if (!fileContent.is_open ())
                               {
                                 return std::unexpected (
                                     std::format ("Cannot open csv file {}",
                                                  csvFile.string ()));
                               }
                             return std::string (
                                 std::istreambuf_iterator<char> (fileContent),
                                 std::istreambuf_iterator<char> ());
                           });
  }

  std::span<std::string> getTestTokens () override { return m_testTokens; }

  std::filesystem::path getReferenceFile () const override
  { return m_reference; }

private:
  voidReturn setUpSubIntervals ()
  {
    fit::WorkoutStepMesg parentMsg = m_wktStep;
    parentMsg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
    parentMsg.SetCustomTargetPowerLow (parentLoInt ());
    parentMsg.SetCustomTargetPowerHigh (parentHiInt ());
    parentMsg.SetDurationTime (parentDur ().count ());
    auto parent{ m_testfileHandler->getInterval (parentMsg) };
    if (!parent)
      {
        return std::unexpected (parent.error ());
      }
    m_testfileHandler->addInterval (std::move (*parent));

    fit::WorkoutStepMesg subMsg = m_wktStep;
    subMsg.SetTargetType (FIT_WKT_STEP_TARGET_POWER);
    subMsg.SetCustomTargetPowerLow (subLoInt ());
    subMsg.SetCustomTargetPowerHigh (subHiInt ());
    subMsg.SetDurationTime (subDur ().count ());
    auto sub{ m_testfileHandler->getInterval (subMsg) };
    if (!sub)
      {
        return std::unexpected (sub.error ());
      }
    m_testfileHandler->addInterval (std::move (*sub));
    return {};
  }

private:
  static constexpr uint16_t AbsolutePowerOffset{ 1000 };
  static constexpr uint16_t AbsoluteHrOffset{ 100 };

  fit::WorkoutStepMesg m_wktStep;
  std::filesystem::path m_testfile{ "Workout.fit" };
  std::filesystem::path m_activity{ "Activity.fit" };
  std::filesystem::path non_existent{ "No_file.fit" };
  std::filesystem::path m_reference{ "Reference.fit" };
  static constexpr std::string_view m_Hash{
    "f904ef284f3385c129c3693a674fd4b2cca8424a7037d41ecc2510a5dfad7d47"
  };
  FitHandler m_nonexistentHandler{ non_existent };
  std::unique_ptr<FitHandler> m_activityHandler{ nullptr };
  std::unique_ptr<FitHandler> m_testfileHandler{ nullptr };
  FitHandler m_referenceHandler{ m_reference };
  std::vector<std::string> m_testTokens;
  std::vector<std::filesystem::path> m_garbage{ m_activity, m_reference,
                                                m_testfile };
};
}; // fitFiles namespace

template <typename ContainerType> class FileTester : public ::testing::Test
{
public:
  void SetUp () override
  {
    // Testfiles have to be generated before initializing the FileHandler,
    // thus this function has to be static

    m_testData = std::make_unique<ContainerType> ();
    this->m_testData->setUp ();
    this->m_testData->setUpIntervals ();
  }
  void TearDown () override { this->m_testData->cleanUp (); }

protected:
  // NOLINTNEXTLINE
  std::unique_ptr<ContainerType> m_testData{ nullptr };
};

TYPED_TEST_SUITE_P (FileTester);

using FitTesterType = ::testing::Types<fitFiles::FitDataTestContainer>;

TYPED_TEST_P (FileTester, InvalidFilesTest)
{
  EXPECT_FALSE (
      this->m_testData->invalidTestFile ().checkFile ().has_value ());
  EXPECT_FALSE (this->m_testData->invalidTestFile ().readFile ().has_value ());
}
TYPED_TEST_P (FileTester, wrongFileContentTest)
{
  EXPECT_TRUE (this->m_testData->wrongFileContent ().checkFile ());
  EXPECT_FALSE (
      this->m_testData->wrongFileContent ().readFile ().has_value ());
}
TYPED_TEST_P (FileTester, WorkoutStepWattsTester)
{
  auto retVal{ this->m_testData->testAbsolutePower () };
  EXPECT_TRUE (retVal);
  EXPECT_EQ (retVal->getIntensity ().getUnitStr (),
             this->m_testData->absoluteUnitString ());
  EXPECT_EQ (*retVal->getIntensity ().getWatts (Level::Low),
             this->m_testData->absolutePowerLo ());
  EXPECT_EQ (*retVal->getIntensity ().getWatts (Level::High),
             this->m_testData->absolutePowerHi ());
}
TYPED_TEST_P (FileTester, WorkoutStepFtpTester)
{
  auto retVal{ this->m_testData->testRelativePower () };
  EXPECT_TRUE (retVal);
  EXPECT_EQ (retVal->getIntensity ().getUnitStr (),
             this->m_testData->relPowerUnitStr ());
  EXPECT_EQ (*retVal->getIntensity ().getPercentFTP (Level::Low),
             this->m_testData->relPowerLo ());
  EXPECT_EQ (*retVal->getIntensity ().getPercentFTP (Level::High),
             this->m_testData->relPowerHi ());
}
TYPED_TEST_P (FileTester, WorkoutStepPwrZoneTester)
{
  auto retVal{ this->m_testData->testPowerZone () };
  EXPECT_TRUE (retVal);
  EXPECT_EQ (retVal->getIntensity ().getUnitStr (),
             this->m_testData->powerZoneUnitStr ());
  EXPECT_EQ (*retVal->getIntensity ().getPowerZone (),
             this->m_testData->powerZone ());
}
TYPED_TEST_P (FileTester, WorkoutStepHrBPMTester)
{
  auto retVal{ this->m_testData->testHrBPM () };
  EXPECT_TRUE (retVal);
  EXPECT_EQ (retVal->getIntensity ().getUnitStr (),
             this->m_testData->absoluteHrUnitString ());
  EXPECT_EQ (*retVal->getIntensity ().getHeartRateBPM (Level::Low),
             this->m_testData->absoluteHrLo ());
  EXPECT_EQ (*retVal->getIntensity ().getHeartRateBPM (Level::High),
             this->m_testData->absoluteHrHi ());
}
TYPED_TEST_P (FileTester, WorkoutStepHrPercentTester)
{
  auto retVal{ this->m_testData->testHrPercentMax () };
  EXPECT_TRUE (retVal);
  EXPECT_EQ (retVal->getIntensity ().getUnitStr (),
             this->m_testData->relHrUnitStr ());
  EXPECT_EQ (*retVal->getIntensity ().getPercentMaxHR (Level::Low),
             this->m_testData->relHrLo ());
  EXPECT_EQ (*retVal->getIntensity ().getPercentMaxHR (Level::High),
             this->m_testData->relHrHi ());
}
TYPED_TEST_P (FileTester, WorkoutStepHrZoneTester)
{

  auto retVal{ this->m_testData->testHrZone () };
  EXPECT_TRUE (retVal);
  EXPECT_EQ (retVal->getIntensity ().getUnitStr (),
             this->m_testData->hrZoneUnitStr ());
  EXPECT_EQ (*retVal->getIntensity ().getHeartRateZone (),
             this->m_testData->hrZone ());
}
TYPED_TEST_P (FileTester, WorkoutStepRepeatMessageTester)
{
  auto repeat{ this->m_testData->testRepeatMessage () };
  ASSERT_FALSE (repeat);
  EXPECT_EQ (repeat.error (), this->m_testData->workoutRepeatStr ());
}
TYPED_TEST_P (FileTester, WorkoutStepInvalidRepeatTester)
{
  auto errMsg{ this->m_testData->testInvalidRepeatMessage () };
  ASSERT_TRUE (errMsg);
  EXPECT_EQ (*errMsg, this->m_testData->illegalMessageRepeatStr ());
}
TYPED_TEST_P (FileTester, WorkoutStepSubIntervalTester)
{
  auto intervals{ this->m_testData->testSubIntervals () };
  ASSERT_TRUE (intervals);
  EXPECT_EQ (intervals->at (0).count (),
             this->m_testData->subIntervalRepeats ());

  auto intervalIt{ intervals->at (0).begin () };

  // First step should be the parent interval
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::Low),
             this->m_testData->parentLoInt ());
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::High),
             this->m_testData->parentHiInt ());
  EXPECT_EQ (intervalIt->getDuration (), this->m_testData->parentDur ());

  // Second step subInterval
  ++intervalIt;
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::Low),
             this->m_testData->subLoInt ());
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::High),
             this->m_testData->subHiInt ());
  EXPECT_EQ (intervalIt->getDuration (), this->m_testData->subDur ());

  // Third step parent interval
  ++intervalIt;
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::Low),
             this->m_testData->parentLoInt ());
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::High),
             this->m_testData->parentHiInt ());
  EXPECT_EQ (intervalIt->getDuration (), this->m_testData->parentDur ());

  // Fourth step subInterval
  ++intervalIt;
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::Low),
             this->m_testData->subLoInt ());
  EXPECT_EQ (*intervalIt->getIntensity ().getPercentFTP (Level::High),
             this->m_testData->subHiInt ());
  EXPECT_EQ (intervalIt->getDuration (), this->m_testData->subDur ());

  // Now it should be the sentinel
  ++intervalIt;
  EXPECT_EQ (intervalIt, intervals->at (0).end ());
}
TYPED_TEST_P (FileTester, WorkoutMsgTester)
{
  auto name{ this->m_testData->testWorkoutName () };
  auto notes{ this->m_testData->testWorkoutNotes () };
  EXPECT_EQ (name, this->m_testData->workoutName ());
  EXPECT_EQ (notes, this->m_testData->workoutNotes ());
}

TYPED_TEST_P (FileTester, FileWriteTest)
{
  auto retVal{ this->m_testData->generateReferenceFile () };
  EXPECT_TRUE (retVal) << retVal.error ();
  EXPECT_EQ (sha256sum (this->m_testData->getReferenceFile ()),
             this->m_testData->getHash ());
}

TYPED_TEST_P (FileTester, FileContentTest)
{

  auto fileContent{ this->m_testData->getFileContent () };
  EXPECT_TRUE (fileContent) << fileContent.error ();

  for (const auto &token : this->m_testData->getTestTokens ())
    {
      EXPECT_TRUE (fileContent->find (token) != std::string::npos)
          << std::format ("Token {} not found in {}", token,
                          this->m_testData->getReferenceFile ().string ());
    }
}

REGISTER_TYPED_TEST_SUITE_P (
    FileTester, InvalidFilesTest, wrongFileContentTest, WorkoutStepWattsTester,
    WorkoutStepFtpTester, WorkoutStepPwrZoneTester, WorkoutStepHrBPMTester,
    WorkoutStepHrPercentTester, WorkoutStepHrZoneTester,
    WorkoutStepRepeatMessageTester, WorkoutStepInvalidRepeatTester,
    WorkoutStepSubIntervalTester, WorkoutMsgTester, FileWriteTest,
    FileContentTest);

INSTANTIATE_TYPED_TEST_SUITE_P (FitFiles, FileTester, FitTesterType);
}; // namespace Workouts