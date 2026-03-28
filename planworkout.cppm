export module planworkout;

import std;
import std.compat;
import workoutlib;

using namespace Workouts;
export class PlanInterval : public Interval
{
public:
  explicit PlanInterval (std::fstream &file, WorkoutType intervalType,
                         ValueRange value, Duration duration)
      : Interval (file, intervalType, value, duration)
  {
  }

  void writeCommon () override;
  void writeAbsoluteWatt () override;
  void writePercentFTP () override;
  void
  writeAbsoluteHeartRate () override
  {
    throw std::runtime_error (
        "No heart rate target possible for .plan files.");
  }
  void
  writePercentMaxHeartRate () override
  {
    throw std::runtime_error (
        "No heart rate target possible for .plan files.");
  }
};

export class PlanWorkout : public Workout
{
public:
  explicit PlanWorkout (const std::string file, std::string workoutName,
                        std::string notes)
      : Workout (file, workoutName, notes)
  {
    wrapDescription (m_notes);
  }

  ~PlanWorkout ()
  {
    writeWorkout ();
    writeToFile ();
  }

  std::size_t createInterval (WorkoutType type, ValueRange value,
                              Duration duration) override;

  void writeWorkout () override;

private:
  void wrapDescription (std::string &string);

private:
  uint32_t m_duration{};
};
