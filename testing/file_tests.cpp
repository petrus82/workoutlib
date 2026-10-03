#include <gtest/gtest.h>

import std;
import workoutlib;
import fitmodule;
import fitfiles;
import textfiles;
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
  0x3c, 0x00, 0x00, 0x00, 0x08, 0x00, 0xca, 0x6f,
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
  0x04, 0x00, 0xb6, 0xf5, 0x44, 0x00, 0xb6, 0xf5, 0x44, 0x01, 0x00, 0xea, 0x43,
};

static constexpr std::string_view PlanFile{
  R"(
=HEADER=

NAME=Workout

DURATION=1260

# TEST COMMENT
PLAN_TYPE=0

# WORKOUT_TYPE=BIKE
WORKOUT_TYPE=0

DESCRIPTION=This is a description
DESCRIPTION=with a second line and äöüÄÖÜß.

=STREAM=

=INTERVAL=
INTERVAL_NAME=WARM UP
HR_LO=60
HR_HI=70
MESG_DURATION_SEC>=300?EXIT

=INTERVAL=
MESG_DURATION_SEC>=0?EXIT
REPEAT=2

=SUBINTERVAL=
PWR_LO=300
PWR_HI=320
MESG_DURATION_SEC>=300?EXIT

=SUBINTERVAL=
PERCENT_FTP_LO=75
PERCENT_FTP_HI=90
MESG_DURATION_SEC>=180?EXIT
)"
};

static constexpr std::string_view ErgFile{ R"(
[COURSE HEADER]
VERSION = 2
UNITS = METRIC
DESCRIPTION = Erg test file.
FILE NAME = ERGTest
MINUTES  WATTS
[END COURSE HEADER]

[COURSE DATA]
0.00 50
6.60 50
6.60 140
7.98 140
[END COURSE DATA]
)" };

static constexpr std::string_view MrcFile{
  R"(
[COURSE HEADER]
VERSION = 2
UNITS = METRIC
DESCRIPTION = A description
FILE NAME = MRCTest
MINUTES PERCENT
[END COURSE HEADER]
[COURSE DATA]
0.00 50
6.60 50
6.60 140
7.98 140
[END COURSE DATA]
)"
};

namespace Workouts
{

struct HasAbsolutePowerTest
{};
struct HasRelativePowerTest
{};
struct HasPowerZoneTest
{};
struct HasHrBPMTest
{};
struct HasHrPercentMaxTest
{};
struct HasHrZoneTest
{};
struct HasRepeatMessageTest
{};
struct HasInvalidRepeatTest
{};
struct HasSubIntervalTest
{};

template <typename Tag, typename... Tags>
struct is_test_supported : std::false_type
{};

template <typename Tag, typename... Rest>
struct is_test_supported<Tag, Tag, Rest...> : std::true_type
{};

template <typename Tag, typename First, typename... Rest>
struct is_test_supported<Tag, First, Rest...> : is_test_supported<Tag, Rest...>
{};

template <typename T, typename Tag>
constexpr bool is_test_supported_v = []<typename... Ts> (std::tuple<Ts...> *)
  {
    return (std::is_same_v<Tag, Ts> || ...);
  }(static_cast<T::supported_tests *> (nullptr));

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
  void cleanUp (std::span<std::filesystem::path> files)
  {
    for (const auto &file : files)
      {
        if (std::filesystem::exists (file))
          {
            std::filesystem::remove (file);
          }
      }
  }
  virtual HandlerType &invalidTestFile () = 0;
  virtual HandlerType &wrongFileContent () = 0;
  virtual std::string testWorkoutName () = 0;
  virtual std::string testWorkoutNotes () = 0;
  virtual intervalReturn testAbsolutePower () { return Interval{}; }
  virtual intervalReturn testRelativePower () { return Interval{}; }
  virtual intervalReturn testPowerZone () { return Interval{}; }
  virtual intervalReturn testHrBPM () { return Interval{}; }
  virtual intervalReturn testHrPercentMax () { return Interval{}; }
  virtual intervalReturn testHrZone () { return Interval{}; }
  virtual std::expected<Intervals, std::string> testSubIntervals ()
  { return Intervals{}; }
  virtual intervalReturn testRepeatMessage () { return Interval{}; }
  virtual stringReturn testInvalidRepeatMessage () { return std::string{}; }
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
    "This is a longer Note with longer lines which have no meaning but some\n"
    "linebreaks and a bunch of crazy characters like these:\n"
    "ÄÖÜäöüß!?.,;:@|<>"
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

  using supported_tests
      = std::tuple<HasAbsolutePowerTest, HasRelativePowerTest,
                   HasPowerZoneTest, HasHrBPMTest, HasHrPercentMaxTest,
                   HasHrZoneTest, HasRepeatMessageTest, HasInvalidRepeatTest,
                   HasSubIntervalTest>;

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
  { DataTestContainer<FitHandler>::cleanUp (m_garbage); }

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
    // specific interval is included. Thus the interval duration in seconds
    // is the number of the interval added. The FitCSVTool returns a csv with
    // the interval duration in fractions of seconds (e.g. "1.0" seconds).
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
                             cmdString.append (" >/dev/null");
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
    "f79ac5697149fc45f2c079a6c023f4b10bc11ced1ccab920693c1de033bc9bd2"
  };
  FitHandler m_nonexistentHandler{ non_existent };
  std::unique_ptr<FitHandler> m_activityHandler{ nullptr };
  std::unique_ptr<FitHandler> m_testfileHandler{ nullptr };
  FitHandler m_referenceHandler{ m_reference };
  std::vector<std::string> m_testTokens;
  std::vector<std::filesystem::path> m_garbage{
    m_activity,
    m_reference,
    m_testfile,
  };
};
}; // namespace fitFiles

namespace textFiles
{

constexpr std::string_view expectedNoteString (R"(
DESCRIPTION = This is a longer Note with longer lines which have no meaning but some
DESCRIPTION = linebreaks and a bunch of crazy characters like these:
DESCRIPTION = ÄÖÜäöüß!?.,;:@|<>
)");

template <typename TextFileHandler>
class TextTestContainer : public DataTestContainer<TextFileHandler>
{
public:
  TextTestContainer () {}
  ~TextTestContainer () override = default;
  TextTestContainer (const TextTestContainer &other) = delete;
  TextTestContainer &operator= (const TextTestContainer &other) = delete;
  TextTestContainer (TextTestContainer &&other) noexcept = default;
  TextTestContainer &operator= (TextTestContainer &&other) = default;

  void setUp () override {}

  void setUpIntervals () override {}
  void cleanUp () override
  { DataTestContainer<TextFileHandler>::cleanUp (m_garbage); }

  TextFileHandler &invalidTestFile () override { return m_nonExistentHandler; }
  TextFileHandler &wrongFileContent () override
  {
    std::ofstream file (m_wrongContent);

    // 1 Mb + 1 byte (above illegal file size for textfiles)
    constexpr const std::ptrdiff_t illegal_size{ (1024 * 1024) + 1 };
    std::vector<char> buffer (illegal_size, 'M');
    file.write (buffer.data (), illegal_size);
    m_wrongContentHandler = std::make_unique<TextFileHandler> (m_wrongContent);
    m_garbage.emplace_back (m_wrongContent);
    return *m_wrongContentHandler;
  }
  stringReturn testInvalidRepeatMessage () override {}
  std::string_view getHash () const override { return m_Hash; }

  voidReturn generateReferenceFile () override
  {

    m_testTokens.emplace_back (m_testfileHandler->fileFormat.headerStart);
    m_testTokens.emplace_back (m_testfileHandler->fileFormat.workoutNameToken);
    m_testTokens.emplace_back (m_testfileHandler->fileFormat.headerSeparator);
    m_testTokens.emplace_back (
        DataTestContainer<TextFileHandler>::workoutName ());
    m_testTokens.emplace_back (expectedNoteString);
    m_testTokens.emplace_back (m_testfileHandler->fileFormat.headerEnd);
    auto intervals{ setupTestIntervals () };
    m_garbage.emplace_back (m_testfile);
    return m_testfileHandler->writeFile (
        m_testfile, DataTestContainer<TextFileHandler>::workoutName (),
        DataTestContainer<TextFileHandler>::workoutNotes (), intervals);
  }

  stringReturn getFileContent () override
  {
    return generateReferenceFile ().and_then (
        [this] () -> stringReturn
          {
            if (!std::filesystem::exists (m_testfile))
              {
                return std::unexpected (
                    std::format ("Cannot find {}", m_testfile.string ()));
              }
            std::ifstream fileContent{ m_testfile, std::ios::in };
            if (!fileContent.is_open ())
              {
                return std::unexpected (
                    std::format ("Cannot open file {}", m_testfile.string ()));
              }
            return std::string (std::istreambuf_iterator<char> (fileContent),
                                std::istreambuf_iterator<char> ());
          });
  }

  std::span<std::string> getTestTokens () override { return m_testTokens; }

  std::string testWorkoutName () override
  {
    if (auto retVal{
            m_testfileHandler->getFileHeader (m_workoutHeaderString),
        };
        retVal)
      {
        return m_testfileHandler->getWorkoutName ();
      }
    return "Test failed.";
  }

  std::string testWorkoutNotes () override
  {
    if (auto retVal{ m_testfileHandler->getFileHeader (m_workoutNoteString) };
        retVal)
      {
        auto out{ m_testfileHandler->getWorkoutNotes () };
        return out;
      }
    return "Test failed.";
  }

protected:
  // Called by the derived classes to set the expected workout name and notes
  // for the checking of the read file functions
  void setupNameTests (std::string_view workoutHeader)
  { m_workoutHeaderString = workoutHeader; }
  void setupNoteTest (std::string_view workoutNotes)
  { m_workoutNoteString = std::string (workoutNotes); }

  // Interface for setting up the interval write tests
  virtual Intervals setupTestIntervals () = 0;

protected:
  // NOLINTBEGIN
  std::vector<std::filesystem::path> m_garbage;
  std::filesystem::path m_testfile{ "testfile.txt" };
  std::unique_ptr<TextFileHandler> m_testfileHandler{
    std::make_unique<TextFileHandler> (m_testfile)
  };
  std::vector<std::string> m_testTokens;
  // NOLINTEND

private:
  std::string m_Hash{ std::string (64, '\0') };
  std::string m_workoutHeaderString;
  std::string m_workoutNoteString;

  std::filesystem::path m_non_existent{ "No_file.txt" };
  std::filesystem::path m_wrongContent{ "wrong.txt" };

  TextFileHandler m_nonExistentHandler{ m_non_existent };
  std::unique_ptr<TextFileHandler> m_wrongContentHandler{};
};

namespace planFiles
{
class PlanTestContainer : public TextTestContainer<PlanHandler>
{
public:
  using supported_tests
      = std::tuple<HasAbsolutePowerTest, HasRelativePowerTest, HasHrBPMTest,
                   HasSubIntervalTest>;

  PlanTestContainer ()
  {
    std::ofstream planStream (m_test);
    if (!planStream.is_open ())
      {
        throw std::runtime_error ("Cannot write testfile.");
      }
    planStream << PlanFile;
    m_garbage.emplace_back (m_test);
    TextTestContainer<PlanHandler>::setupNameTests (R"(
=HEADER=

NAME=Workout

DURATION=1250

# TEST COMMENT
PLAN_TYPE=0

# WORKOUT_TYPE=BIKE
WORKOUT_TYPE=0

=STREAM=
)");
    TextTestContainer<PlanHandler>::setupNoteTest (expectedNoteString);
  }

  intervalReturn testAbsolutePower () override
  {
    std::string testString{
      std::format ("=INTERVAL=\n"
                   "PWR_LO={}\n"
                   "PWR_HI={}\n"
                   "MESG_DURATION_SEC>={}?EXIT",
                   absolutePowerLo (), absolutePowerHi (),
                   parentDur ().count ()),
    };
    return m_testHandler.getIntervalStrings (testString)->front ();
  }
  intervalReturn testRelativePower () override
  {
    std::string testString{
      std::format ("PERCENT_FTP_LO={}\n"
                   "PERCENT_FTP_HI={}\n"
                   "MESG_DURATION_SEC>={}",
                   relPowerLo (), relPowerHi (),
                   std::chrono::seconds (1).count ()),
    };
    return m_testHandler.getIntervalStrings (testString)->front ();
  }
  intervalReturn testHrBPM () override
  {
    std::string testString{
      std::format ("HR_LO={}\n"
                   "HR_HI={}\n"
                   "MESG_DURATION_SEC>={}?EXIT",
                   absoluteHrLo (), absoluteHrHi (),
                   std::chrono::seconds (1).count ()),
    };
    return m_testHandler.getIntervalStrings (testString)->front ();
  }
  std::expected<Intervals, std::string> testSubIntervals () override
  {
    // subIntervalRepeats has to be divided by 2 because the interval iterator
    // counts the parent intervals as well
    std::string testString{
      std::format ("REPEAT={}\n\n"
                   "=SUBINTERVAL=\n"
                   "PERCENT_FTP_LO={}\n"
                   "PERCENT_FTP_HI={}\n"
                   "MESG_DURATION_SEC>={}?EXIT\n\n"
                   "=SUBINTERVAL=\n"
                   "PERCENT_FTP_LO={}\n"
                   "PERCENT_FTP_HI={}\n"
                   "MESG_DURATION_SEC>={}?EXIT\n\n",
                   subIntervalRepeats () / 2, parentLoInt (), parentHiInt (),
                   parentDur ().count (), subLoInt (), subHiInt (),
                   subDur ().count ()),
    };
    auto repeat{ m_testHandler.getIntervalStrings (testString) };
    if (!repeat)
      {
        return std::unexpected (repeat.error ());
      }
    return *repeat;
  }

  std::filesystem::path getReferenceFile () const override
  { return std::filesystem::path ("Testfile.plan"); }

private:
  Intervals setupTestIntervals () override
  {
    Intervals intervals{};
    const auto totalDuration{ ((1 + 2) * 2) + 3 + 4 };
    m_testTokens.emplace_back (std::format ("DURATION={}", totalDuration));
    Interval parent{ Intensity{ IntensityPair{ absolutePowerLo (),
                                               absolutePowerHi () },
                                IntensityUnit::Watts, ftp () },
                     std::chrono::seconds (1) };

    parent.addSubInterval (
        Interval{ Intensity{ IntensityPair{ subLoInt (), subHiInt () },
                             IntensityUnit::Watts, ftp () },
                  std::chrono::seconds (2) });
    parent.addRepeat (Repeat{ .begin = -1, .end = 0, .times = 2 });
    intervals.emplace_back (std::move (parent));
    m_testTokens.emplace_back ("=INTERVAL=");
    m_testTokens.emplace_back (
        std::format ("REPEAT={}", subIntervalRepeats () / 2));
    m_testTokens.emplace_back ("=SUBINTERVAL=");
    m_testTokens.emplace_back (std::format ("PWR_LO={}", absolutePowerLo ()));
    m_testTokens.emplace_back (std::format ("PWR_HI={}", absolutePowerHi ()));
    m_testTokens.emplace_back (std::format ("MESG_DURATION_SEC>={}", 1));
    m_testTokens.emplace_back ("=SUBINTERVAL=");
    m_testTokens.emplace_back (std::format ("PWR_LO={}", subLoInt ()));
    m_testTokens.emplace_back (std::format ("PWR_HI={}", subHiInt ()));
    m_testTokens.emplace_back (std::format ("MESG_DURATION_SEC>={}", 2));

    intervals.emplace_back (
        Interval{ Intensity{ IntensityPair{ relPowerLo (), relPowerHi () },
                             IntensityUnit::PercentFTP, ftp () },
                  std::chrono::seconds (3) });
    m_testTokens.emplace_back ("=SUBINTERVAL=");
    m_testTokens.emplace_back (
        std::format ("PERCENT_FTP_LO={}", relPowerLo ()));
    m_testTokens.emplace_back (
        std::format ("PERCENT_FTP_HI={}", relPowerHi ()));
    m_testTokens.emplace_back (std::format ("MESG_DURATION_SEC>={}", 3));

    intervals.emplace_back (
        Interval{ Intensity{ IntensityPair{ absoluteHrLo (), absoluteHrHi () },
                             IntensityUnit::HeartRateBPM, ftp () },
                  std::chrono::seconds (4) });
    m_testTokens.emplace_back ("=SUBINTERVAL=");
    m_testTokens.emplace_back (std::format ("HR_LO={}", absoluteHrLo ()));
    m_testTokens.emplace_back (std::format ("HR_HI={}", absoluteHrHi ()));
    m_testTokens.emplace_back (std::format ("MESG_DURATION_SEC>={}", 4));
    return intervals;
  }

private:
  std::filesystem::path m_test{ "Test.plan" };
  PlanHandler m_testHandler{ m_test };
};
}; // namespace planFiles

constexpr std::string_view expectedHeaderString{
  R"(
[COURSE HEADER]
VERSION = 2\
UNITS = METRIC
FILE NAME = Workout
    )"
};

namespace ergFiles
{
class ErgTestContainer : public TextTestContainer<ErgHandler>
{
public:
  using supported_tests = std::tuple<HasAbsolutePowerTest>;
  explicit ErgTestContainer ()
  {
    TextTestContainer<ErgHandler>::setupNameTests (expectedHeaderString);
    TextTestContainer<ErgHandler>::setupNoteTest (expectedNoteString);
  }

  std::filesystem::path getReferenceFile () const override
  { return std::filesystem::path ("Testfile.erg"); }

  intervalReturn testAbsolutePower () override
  {
    std::string testString{
      std::format ("[COURSE DATA]\n"
                   "0.00\t{}\n"
                   "5.00\t{}\n"
                   "[END COURSE DATA]\n",
                   absolutePowerLo (), absolutePowerLo ()),
    };
    auto interval{
      m_testfileHandler->getIntervalStrings (testString)->front ()
    };
    // Erg files don't support an intensity range, so it has to be set here
    interval.setIntensity (Intensity{ absolutePowerHi (), IntensityUnit::Watts,
                                      ftp (), Level::High });
    return interval;
  }

private:
private:
  Intervals setupTestIntervals () override
  {
    Intervals intervals{};

    Interval parent{ Intensity{ IntensityPair{ absolutePowerLo (),
                                               absolutePowerLo () },
                                IntensityUnit::Watts, ftp () },
                     std::chrono::seconds (300) };
    parent.addSubInterval (
        Interval{ Intensity{ IntensityPair{ subLoInt (), subLoInt () },
                             IntensityUnit::Watts, ftp () },
                  std::chrono::seconds (300) });
    parent.addRepeat (Repeat{ .begin = -1, .end = 0, .times = 2 });
    intervals.emplace_back (std::move (parent));
    m_testTokens.emplace_back ("[COURSE DATA]");
    m_testTokens.emplace_back (std::format ("0.00\t{}", absolutePowerLo ()));
    m_testTokens.emplace_back (std::format ("5.00\t{}", absolutePowerLo ()));
    m_testTokens.emplace_back (std::format ("5.00\t{}", subLoInt ()));
    m_testTokens.emplace_back (std::format ("10.00\t{}", subLoInt ()));
    m_testTokens.emplace_back (std::format ("10.00\t{}", absolutePowerLo ()));
    m_testTokens.emplace_back (std::format ("15.00\t{}", absolutePowerLo ()));
    m_testTokens.emplace_back (std::format ("15.00\t{}", subLoInt ()));
    m_testTokens.emplace_back (std::format ("20.00\t{}", subLoInt ()));
    m_testTokens.emplace_back ("[END COURSE DATA]");

    return intervals;
  }
};
}; // namespace ergFiles

namespace mrcFiles
{
class MrcTestContainer : public TextTestContainer<MrcHandler>
{
public:
  using supported_tests = std::tuple<HasRelativePowerTest>;
  explicit MrcTestContainer ()
  {
    TextTestContainer<MrcHandler>::setupNameTests (expectedHeaderString);
    TextTestContainer<MrcHandler>::setupNoteTest (expectedNoteString);
  }

  std::filesystem::path getReferenceFile () const override
  { return std::filesystem::path ("Testfile.mrc"); }

  intervalReturn testRelativePower () override
  {
    std::string testString{
      std::format ("[COURSE DATA]\n"
                   "0.00\t{}\n"
                   "5.00\t{}\n"
                   "[END COURSE DATA]\n",
                   relPowerLo (), relPowerLo ()),
    };
    auto interval{
      m_testfileHandler->getIntervalStrings (testString)->front ()
    };
    // Mrc files don't support an intensity range, so it has to be set here
    interval.setIntensity (Intensity{ relPowerHi (), IntensityUnit::PercentFTP,
                                      ftp (), Level::High });
    return interval;
  }

private:
  Intervals setupTestIntervals () override
  {
    Intervals intervals{};

    Interval parent{
      Interval{ Intensity{ IntensityPair{ relPowerHi (), relPowerHi () },
                           IntensityUnit::PercentFTP, ftp () },
                std::chrono::seconds (300) },
    };

    parent.addSubInterval (
        Interval{ Intensity{ IntensityPair{ subLoInt (), subLoInt () },
                             IntensityUnit::PercentFTP, ftp () },
                  std::chrono::seconds (300) });
    parent.addRepeat (Repeat{ .begin = -1, .end = 0, .times = 2 });
    intervals.emplace_back (std::move (parent));
    m_testTokens.emplace_back ("[COURSE DATA]");
    m_testTokens.emplace_back (std::format ("0.00\t{}", relPowerHi ()));
    m_testTokens.emplace_back (std::format ("5.00\t{}", relPowerHi ()));
    m_testTokens.emplace_back (std::format ("5.00\t{}", subLoInt ()));
    m_testTokens.emplace_back (std::format ("10.00\t{}", subLoInt ()));
    m_testTokens.emplace_back (std::format ("10.00\t{}", relPowerHi ()));
    m_testTokens.emplace_back (std::format ("15.00\t{}", relPowerHi ()));
    m_testTokens.emplace_back (std::format ("15.00\t{}", subLoInt ()));
    m_testTokens.emplace_back (std::format ("20.00\t{}", subLoInt ()));
    m_testTokens.emplace_back ("[END COURSE DATA]");
    return intervals;
  }
};
}; // namespace mrcFiles

}; // namespace textFiles

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
using PlanTesterType
    = ::testing::Types<textFiles::planFiles::PlanTestContainer>;
using ErgTesterType = ::testing::Types<textFiles::ergFiles::ErgTestContainer>;
using MrcTesterType = ::testing::Types<textFiles::mrcFiles::MrcTestContainer>;

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
  if constexpr (!is_test_supported_v<TypeParam, HasAbsolutePowerTest>)
    {
      GTEST_SKIP () << "Absolute Power Test not supported.";
    }
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
  if constexpr (!is_test_supported_v<TypeParam, HasRelativePowerTest>)
    {
      GTEST_SKIP () << "Relative Power Test not supported.";
    }
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
  if constexpr (!is_test_supported_v<TypeParam, HasPowerZoneTest>)
    {
      GTEST_SKIP () << "Power Zone Test not supported.";
    }
  auto retVal{ this->m_testData->testPowerZone () };
  EXPECT_TRUE (retVal);
  EXPECT_EQ (retVal->getIntensity ().getUnitStr (),
             this->m_testData->powerZoneUnitStr ());
  EXPECT_EQ (*retVal->getIntensity ().getPowerZone (),
             this->m_testData->powerZone ());
}
TYPED_TEST_P (FileTester, WorkoutStepHrBPMTester)
{
  if constexpr (!is_test_supported_v<TypeParam, HasHrBPMTest>)
    {
      GTEST_SKIP () << "HrBPM Test not supported.";
    }

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
  if constexpr (!is_test_supported_v<TypeParam, HasHrPercentMaxTest>)
    {
      GTEST_SKIP () << "HrPercentMax Test not supported.";
    }

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
  if constexpr (!is_test_supported_v<TypeParam, HasHrZoneTest>)
    {
      GTEST_SKIP () << "HrZone Test not supported.";
    }

  auto retVal{ this->m_testData->testHrZone () };
  EXPECT_TRUE (retVal);
  EXPECT_EQ (retVal->getIntensity ().getUnitStr (),
             this->m_testData->hrZoneUnitStr ());
  EXPECT_EQ (*retVal->getIntensity ().getHeartRateZone (),
             this->m_testData->hrZone ());
}
TYPED_TEST_P (FileTester, WorkoutStepRepeatMessageTester)
{
  if constexpr (!is_test_supported_v<TypeParam, HasRepeatMessageTest>)
    {
      GTEST_SKIP () << "RepeatMessage Test not supported.";
    }
  auto repeat{ this->m_testData->testRepeatMessage () };
  ASSERT_FALSE (repeat);
  EXPECT_EQ (repeat.error (), this->m_testData->workoutRepeatStr ());
}
TYPED_TEST_P (FileTester, WorkoutStepInvalidRepeatTester)
{
  if constexpr (!is_test_supported_v<TypeParam, HasInvalidRepeatTest>)
    {
      GTEST_SKIP () << "Invalid Repeat Test not supported.";
    }

  auto errMsg{ this->m_testData->testInvalidRepeatMessage () };
  ASSERT_TRUE (errMsg);
  EXPECT_EQ (*errMsg, this->m_testData->illegalMessageRepeatStr ());
}
TYPED_TEST_P (FileTester, WorkoutStepSubIntervalTester)
{
  if constexpr (!is_test_supported_v<TypeParam, HasSubIntervalTest>)
    {
      GTEST_SKIP () << "SubInterval Test not supported.";
    }

  auto intervals{ this->m_testData->testSubIntervals () };
  ASSERT_TRUE (intervals) << intervals.error ();
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
  if (fileContent)
    {
      auto testTokens{ this->m_testData->getTestTokens () };
      EXPECT_GT (testTokens.size (), 0) << "No test tokens available.";
      bool foundAllTokens{ true };
      for (const auto &token : this->m_testData->getTestTokens ())
        {
          if (auto result{ fileContent->find (token) };
              result == std::string::npos)
            {
              std::println ("Token {}\n not found", token);
              foundAllTokens = false;
            }
        }
      if (!foundAllTokens)
        {
          FAIL () << "File content: \n"
                  << "*************************\n"
                  << *fileContent << "\n" //
                  << "*************************\n";
        }
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
INSTANTIATE_TYPED_TEST_SUITE_P (PlanFiles, FileTester, PlanTesterType);
INSTANTIATE_TYPED_TEST_SUITE_P (ErgFiles, FileTester, ErgTesterType);
INSTANTIATE_TYPED_TEST_SUITE_P (MrcFiles, FileTester, MrcTesterType);

namespace textFiles
{

TEST (ErgMrcTests, BlockTest)
{
  // 0 elements
  std::vector<int> blockLen;
  {
    auto block{ generateBlock (blockLen) };
    auto blockIt{ block.begin () };
    EXPECT_EQ (blockIt, block.end ());
  }
  { // 1 element
    blockLen.emplace_back (1);
    auto block{ generateBlock (blockLen) };
    auto blockIt{ block.begin () };
    EXPECT_EQ (block.size (), 0);
    EXPECT_EQ (blockIt, block.end ());
  }
  {
    // 2 elements
    blockLen.emplace_back (1);
    auto block{ generateBlock (blockLen) };
    auto blockIt{ block.begin () };
    EXPECT_EQ (block.size (), 1);
    EXPECT_EQ (*blockIt++, 1);
  }
  {
    // 3 elements
    blockLen.emplace_back (1);
    auto block{ generateBlock (blockLen) };
    auto blockIt{ block.begin () };
    EXPECT_EQ (*blockIt++, 1);
    EXPECT_EQ (block.size (), 1);
  }
  {
    // 4 elements
    blockLen.emplace_back (1);
    auto block{ generateBlock (blockLen) };
    auto blockIt{ block.begin () };
    EXPECT_EQ (*blockIt++, 1);
    EXPECT_EQ (*blockIt++, 2);
    EXPECT_EQ (block.size (), 2);
  }

  { // 5 elements
    blockLen.emplace_back (1);
    auto block{ generateBlock (blockLen) };
    auto blockIt{ block.begin () };
    EXPECT_EQ (*blockIt++, 1);
    EXPECT_EQ (*blockIt++, 2);
    EXPECT_EQ (block.size (), 2);
  }

  { // 6 elements
    blockLen.emplace_back (1);
    auto block{ generateBlock (blockLen) };
    auto blockIt{ block.begin () };
    EXPECT_EQ (*blockIt++, 1);
    EXPECT_EQ (*blockIt++, 2);
    EXPECT_EQ (*blockIt++, 3);
    EXPECT_EQ (block.size (), 3);
  }
}
auto generateTestData (std::span<int> intensities)
{
  return intensities
         | std::views::transform (
             [] (uint16_t intensity) -> Interval
               {
                 return Interval{ Intensity{ intensity, IntensityUnit::Watts,
                                             200 },
                                  std::chrono::seconds (1) };
               })
         | std::ranges::to<std::vector<Interval>> ();
}

constexpr void checkTestData (std::span<Interval> test, std::span<int> expLong,
                              std::span<int> expShort)
{
  EXPECT_EQ (test.size (), expShort.size ());
  for (auto &&[result, expected] : std::views::zip (test, expShort))
    {
      EXPECT_EQ (*result.getIntensity ().getWatts (), expected);
    }
  std::vector<uint16_t> results;
  std::ranges::for_each (
      test,
      [&results] (const auto &interval)
        {
          std::ranges::for_each (
              interval,
              [&results] (auto subInterval)
                {
                  results.emplace_back (
                      *subInterval.getIntensity ().getWatts ());
                });
        });
  if (!std::ranges::equal (results, expLong))
    {
      std::println ("Results ({}) are not equal to test data: {} ", results,
                    expLong);
      std::println ("Compressed intervals: {}", test);
      FAIL ();
    }
}
TEST (ErgMrcTests, repeatTests)
{
  {
    // No repeating
    std::array testDataLong{ 1, 2, 3 };
    auto test = generateTestData (testDataLong);
    std::array expected{ 1, 2, 3 };
    blockEncode (test);
    checkTestData (test, testDataLong, expected);
  }
  {
    // Repeating of blockLength 1
    std::array testDataLong{ 2, 2 };
    auto test = generateTestData (testDataLong);
    std::array expected{ 2 };
    blockEncode (test);
    checkTestData (test, testDataLong, expected);
  }
  {
    // Repeating of blockLength 2 with repeat count of 3
    std::array testDataLong{ 1, 2, 3, 2, 3, 2, 3, 4 };
    auto test = generateTestData (testDataLong);
    std::array expected{ 1, 2, 4 };
    blockEncode (test);
    checkTestData (test, testDataLong, expected);
  }
  {
    // Repeating of blockLength 2 and blockLength 5
    std::array testDataLong{ 1, 2, 3, 2, 3, 4, 2, 3, 2, 3, 4, 5 };
    auto test = generateTestData (testDataLong);
    std::array expected{ 1, 2, 5 };
    blockEncode (test);
    checkTestData (test, testDataLong, expected);
  }
}

}; // namespace textFiles
}; // namespace Workouts

int main (int argc, char **argv)
{
  testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}