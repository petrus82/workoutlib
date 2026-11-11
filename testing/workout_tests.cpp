#include <gtest/gtest.h>

import std;
import workoutlib;
import textfiles;

using namespace std::chrono_literals;

namespace Workouts
{

// ============================================================================
// Test Fixture: WorkoutIntegrationTest
// ============================================================================

/**
 * @brief Integration test fixture for the Workout class.
 *
 * This fixture provides:
 * - Embedded ERG, MRC, and Plan file content for testing
 * - Helper functions to create temporary files from string content
 * - FTP and intensity constants used across all tests
 */
class WorkoutIntegrationTest : public ::testing::Test
{
protected:
    // FTP and intensity constants
    static constexpr uint16_t ftp{ 300 };
    static constexpr uint16_t powerLow{ 150 };   // 50% FTP
    static constexpr uint16_t relLow{ 50 };     // 50% FTP
    static constexpr uint16_t powerHigh{ 300 };  // 100% FTP
    static constexpr uint16_t relHigh{ 100 };  // 100% FTP
    static constexpr uint16_t repeats {2};

    static constexpr std::chrono::seconds warmupDuration{ 600 };  // 10 minutes
    static constexpr std::chrono::minutes warmupMin{ 10 };  // 10 minutes
    static constexpr std::chrono::seconds intervalDuration{ 300 }; // 5 minutes
    static constexpr std::chrono::minutes intervalMin { 5 }; // 5 minutes
    std::string_view workoutName {"Workout Name"};
    static constexpr std::string_view workoutNotes{
        "This is a longer Note with longer lines which have no meaning but some\n"
        "linebreaks and a bunch of crazy characters like these:\n"
        "ÄÖÜäöüß!?.,;:@|<>"
    };
    const std::string ergFileContent {std::format(
        (
    "[COURSE HEADER]\n"
    "FILE NAME = {}\n"
    "DESCRIPTION = This is a longer Note with longer lines which have no meaning but some\n"
    "DESCRIPTION = linebreaks and a bunch of crazy characters like these:\n"
    "DESCRIPTION = ÄÖÜäöüß!?.,;:@|<>\n"
    "FTP = {}\n"
    "MINUTES WATTS\n"
    "[END COURSE HEADER]\n"
    "[COURSE DATA]\n"
    "{}\t{}\n"
    "{}\t{}\n"
    "{}\t{}\n"
    "{}\t{}\n"
    "[END COURSE DATA]\n"
    ), 
    workoutName,
    std::to_string(ftp),
    0, powerLow, 
    warmupMin.count(), powerLow, 
    warmupMin.count(), powerHigh,
    warmupMin.count() + intervalMin.count(), powerHigh)};

    const std::string mrcFileContent {std::format(
        (
    "[COURSE HEADER]\n"
    "FILE NAME = {}\n"
    "DESCRIPTION = This is a longer Note with longer lines which have no meaning but some\n"
    "DESCRIPTION = linebreaks and a bunch of crazy characters like these:\n"
    "DESCRIPTION = ÄÖÜäöüß!?.,;:@|<>\n"
    "MINUTES PERCENT\n"
    "[END COURSE HEADER]\n"
    "[COURSE DATA]\n"
    "{}\t{}\n"
    "{}\t{}\n"
    "{}\t{}\n"
    "{}\t{}\n"
    "[END COURSE DATA]\n"
    ), 
    workoutName,
    0, relLow, 
    warmupMin.count(), relLow, 
    warmupMin.count(), relHigh, 
    warmupMin.count() + intervalMin.count(), relHigh)};

    // ============================================================================
    // Embedded Plan File Content
    // ============================================================================
    // Plan files use =HEADER=/=STREAM=/=INTERVAL= markers:
    // - =HEADER= section contains NAME, DESCRIPTION, etc.
    // - =STREAM= separates header from intervals
    // - =INTERVAL= markers delimit individual intervals
    // - Each interval has key=value pairs:
    //   - MESG_DURATION_SEC>=X?EXIT: duration with exit condition
    //   - REPEAT=N: number of repetitions
    //   - PWR_LO/PWR_HI: power range (Watts)
    //   - PERCENT_FTP_LO/PERCENT_FTP_HI: power range (%FTP)
    //   - HR_LO/HR_HI: heart rate range (BPM)
    const std::string planFileContent {std::format(
    "=HEADER=\n"
    "NAME={}\n"
    "DURATION={}\n"
    "DESCRIPTION = This is a longer Note with longer lines which have no meaning but some\n"
    "DESCRIPTION = linebreaks and a bunch of crazy characters like these:\n"
    "DESCRIPTION = ÄÖÜäöüß!?.,;:@|<>\n"
    "WORKOUT_TYPE=0\n"
    "\n=STREAM=\n"
    "\n=INTERVAL=\n"
    "REPEAT={}\n"
    "\n=SUBINTERVAL=\n"
    "PWR_LO={}\n"
    "PWR_HI={}\n"
    "MESG_DURATION_SEC>={}?EXIT\n"
    "\n=SUBINTERVAL=\n"
    "PERCENT_FTP_LO={}\n"
    "PERCENT_FTP_HI={}\n"
    "MESG_DURATION_SEC>={}?EXIT\n",
    workoutName,
    std::to_string((warmupDuration.count()+intervalDuration.count()) * repeats),
    repeats,
    powerLow,
    powerHigh,
    warmupDuration.count(),
    relLow,
    relHigh,
    intervalDuration.count())};

  // Temporary file tracking
  std::vector<std::filesystem::path> m_tempFiles;

  // ============================================================================
  // Helper Functions
  // ============================================================================

  /**
   * @brief Create a temporary file with the given content and extension.
   * @param content The file content to write.
   * @param extension The file extension (e.g., ".fit", ".erg", ".mrc", ".plan").
   * @return std::filesystem::path to the created temporary file.
   * @throws std::runtime_error if file creation fails.
   */
  std::filesystem::path
  createTempFile (std::string_view content, std::string_view extension)
  {
    std::string tempPath = std::format ("/tmp/workout_test_{}{}",
                                        std::rand (), extension);
    std::ofstream file (tempPath);
    if (!file.is_open ())
      {
        throw std::runtime_error (
            std::format ("Failed to create temporary file: {}", tempPath));
      }
    file << content;
    file.close ();
    m_tempFiles.push_back (tempPath);
    return tempPath;
  }

  /**
   * @brief Clean up all temporary files created during the test.
   */
  void
  TearDown () override
  {
    for (const auto &path : m_tempFiles)
      {
        if (std::filesystem::exists (path))
          {
            std::filesystem::remove (path);
          }
      }
    m_tempFiles.clear ();
  }

  /**
   * @brief Verify that an expected result is valid and return the value.
   * @param expected The std::expected to check.
   * @param description Description of what was being tested (for error messages).
   * @return The value inside the expected on success.
   */
  template <typename T>
  T &
  expectValid (std::expected<T, std::string> &expected,
               std::string_view description)
  {
    ASSERT_TRUE (expected.has_value ())
        << "Expected success for: " << description
        << ", but got error: " << expected.error ();
    return expected.value ();
  }

  /**
   * @brief Verify that an expected result has an error.
   * @param expected The std::expected to check.
   * @param description Description of what was being tested (for error messages).
   */
  template <typename T>
  void
  expectError (std::expected<T, std::string> &expected,
               std::string_view description)
  {
    ASSERT_FALSE (expected.has_value ())
        << "Expected error for: " << description
        << ", but got success with value: "
        << (expected.has_value () ? expected.value () : "void");
  }
};

TEST_F (WorkoutIntegrationTest, ErgFileCreatesValidWorkout)
{
    std::filesystem::path tempPath = createTempFile (ergFileContent, ".erg");
    textFiles::ergFiles::ErgHandler handler (tempPath);
    std::expected<Workout, std::string> result = readFile (handler);

    // Verify Workout was created successfully
    ASSERT_TRUE (result.has_value ())
        << "ERG file parsing failed: " << result.error ();

    const Workout &workout = result.value ();

    // Verify workout name was parsed from file header
    ASSERT_EQ (handler.getWorkoutName (), workoutName)
        << "Workout name should match ERG file header";

    ASSERT_EQ (handler.getWorkoutNotes(), workoutNotes)
        << "Workout notes should match";

    // Verify FTP was set correctly
    ASSERT_EQ (workout.getFtp (), ftp)
        << "FTP should match value from ERG file header";

    // Verify intervals were parsed from cumulative duration format
    auto intervals = workout.getIntervals ();
    ASSERT_FALSE (intervals.empty ())
        << "ERG file should produce at least one interval";

    // Verify interval count matches expected number of phases
    // ERG file has 4 cumulative data points creating 3 phases
    ASSERT_EQ (intervals.size (), 2u)
        << "ERG file should contain 2 intervals";
    
    // Check units
    for (const auto& interval : intervals)
    {
        ASSERT_EQ (interval.getIntensity().getType(), IntensityUnit::Watts)
        << "Units wrong in ERG file.";
    }

    // Check duration and intensity
    ASSERT_EQ(*intervals[0].getIntensity().getWatts(), powerLow)
        << "First interval doesn't have the correct intensity";
    ASSERT_EQ(intervals[0].getDuration(), warmupDuration)
        << "First interval doesn't have the correct duration";
    ASSERT_EQ(*intervals[1].getIntensity().getWatts(), powerHigh)
        << "First interval doesn't have the correct intensity";
    ASSERT_EQ(intervals[1].getDuration(), intervalDuration)
        << "First interval doesn't have the correct duration";
}

TEST_F (WorkoutIntegrationTest, MrcFileCreatesValidWorkout)
{
    std::filesystem::path tempPath = createTempFile (mrcFileContent, ".mrc");
    textFiles::mrcFiles::MrcHandler handler (tempPath);
    std::expected<Workout, std::string> result = readFile (handler);

    // Verify Workout was created successfully
    ASSERT_TRUE (result.has_value ())
        << "MRC file parsing failed: " << result.error ();

    const Workout &workout = result.value ();

    // Verify workout name was parsed from file header
    ASSERT_EQ (handler.getWorkoutName (), workoutName)
        << "Workout name should match MRC file header";

    ASSERT_EQ (handler.getWorkoutNotes(), workoutNotes)
        << "Workout notes should match";

    // Verify intervals were parsed from cumulative duration format
    auto intervals = workout.getIntervals ();
    ASSERT_FALSE (intervals.empty ())
        << "MRC file should produce at least one interval";

    // Verify interval count matches expected number of phases
    ASSERT_EQ (intervals.size (), 2u)
        << "MRC file should contain 2 intervals";

    // Verify intensity unit is PercentFTP for MRC files
    for (const Interval &interval : intervals)
    {
        ASSERT_EQ (interval.getIntensity ().getType (), IntensityUnit::PercentFTP)
            << "MRC file intervals should use PercentFTP intensity unit";
    }

    // Check duration and intensity
    ASSERT_EQ(*intervals[0].getIntensity().getPercentFTP(), relLow)
        << "First interval doesn't have the correct intensity";
    ASSERT_EQ(intervals[0].getDuration(), warmupDuration)
        << "First interval doesn't have the correct duration";
    ASSERT_EQ(*intervals[1].getIntensity().getPercentFTP(), relHigh)
        << "First interval doesn't have the correct intensity";
    ASSERT_EQ(intervals[1].getDuration(), intervalDuration)
        << "First interval doesn't have the correct duration";
}

TEST_F (WorkoutIntegrationTest, PlanFileTests)
{
    std::filesystem::path tempPath = createTempFile (planFileContent, ".plan");
    textFiles::planFiles::PlanHandler handler (tempPath);
    std::expected<Workout, std::string> result = readFile (handler);

    // Verify Workout was created successfully
    ASSERT_TRUE (result.has_value ())
        << "Plan file parsing failed: " << result.error ();

    const Workout &workout = result.value ();

    // Verify workout name was parsed from file header
    ASSERT_EQ (handler.getWorkoutName (), workoutName)
        << "Workout name should match Plan file header";

    // Check Notes
    ASSERT_EQ (handler.getWorkoutNotes(), workoutNotes)
        << "Workout notes should match";

    // Verify intervals were parsed from Plan file format
    auto intervals = workout.getIntervals ();
    ASSERT_FALSE (intervals.empty ())
        << "Plan file should have intervals";

    // Verify interval count matches expected number of intervals
    ASSERT_EQ (intervals.size (), 1u)
        << "Plan file should have 1 interval";
    ASSERT_EQ (intervals[0].getSubIntervals().size(), 1)
        << "Plan file should have 1 subInterval";

    // Check repeats
    const auto& repVec = intervals[0].getRepeats();
    ASSERT_EQ(repVec.size(), 1)
        << "Plan should have 1 Repeat";
    
    ASSERT_EQ(repVec.at(0).times, repeats)
        << std::format("Plan doesn't have {}", repeats);
    ASSERT_EQ(repVec.at(0).begin, -1)
        << "Repeat doesn't start at parent interval";
    ASSERT_EQ(repVec.at(0).end, 0)
        << "Repeat should include subInterval";

    // Verify first interval
    ASSERT_EQ (intervals[0].getIntensity ().getType (), IntensityUnit::Watts)
        << "First interval should use Watts";
    ASSERT_EQ (intervals[0].getDuration(), warmupDuration)
        << "Incorrect duration for first interval";
    ASSERT_EQ(*intervals[0].getIntensity().getWatts(Level::Low), powerLow)
        << "Incorrect intensity for first interval";
    ASSERT_EQ(*intervals[0].getIntensity().getWatts(Level::High), powerHigh)
        << "Incorrect intensity for first interval";
    ASSERT_EQ(intervals [0].getDuration(), warmupDuration)
        << "Incorrect duration for fist interval";

    // Verify subInterval
    auto subInterval {intervals[0].subIntervalAt(1)};
    ASSERT_EQ (subInterval.getIntensity ().getType (), IntensityUnit::PercentFTP)
        << "Second interval should use PercentFTP";
    ASSERT_EQ(*subInterval.getIntensity().getPercentFTP(Level::Low), relLow)
        << "Second interval has incorrect intensity for Level::Low";
    ASSERT_EQ(*subInterval.getIntensity().getPercentFTP(Level::High), relHigh)
        << "Second interval has incorrect intensity for Level::High";
    ASSERT_EQ(subInterval.getDuration(), intervalDuration)
        << "Second interval has incorrect duration";

}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F (WorkoutIntegrationTest, NonExistentFileReturnsError)
{
  // Test with non-existent file path
  std::filesystem::path nonExistentPath = "/nonexistent/path/workout.erg";
  textFiles::ergFiles::ErgHandler handler (nonExistentPath);
  std::expected<Workout, std::string> result = readFile (handler);

  ASSERT_FALSE (result.has_value ())
      << "Should return error for non-existent file";
  ASSERT_EQ (result.error (), "File /nonexistent/path/workout.erg does not exist.")
      << "Error message should indicate file not found";
}

TEST_F (WorkoutIntegrationTest, InvalidFileFormatReturnsError)
{
  // Create a file with invalid content (not matching any known format)
  std::string invalidContent = "This is not a valid workout file\nRandom content\n";
  std::filesystem::path tempPath = createTempFile (invalidContent, ".erg");

  textFiles::ergFiles::ErgHandler handler (tempPath);
  std::expected<Workout, std::string> result = readFile (handler);

  ASSERT_FALSE (result.has_value ())
      << "Should return error for invalid file format";
}

} // namespace Workouts
