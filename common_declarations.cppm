export module common;

import config;
import std;
import std.compat;

namespace Workouts
{

// A shorter version of std::to_underlying to make the code a bit shorter
export template <class Enum>
constexpr auto
enumVal (Enum e) noexcept // NOLINT(readability-identifier-naming)
{ return std::to_underlying (e); }

export enum HRZ : uint8_t {
  H1 = 1, // 50-60% max heart rate
  H2 = 2, // 61-70% max heart rate
  H3 = 3, // 71-80% max heart rate
  H4 = 4, // 81-90% max heart rate
  H5 = 5  // 91-100% max heart rate
};

export using ZonePair
    = std::pair<uint8_t, uint8_t>; // Low and high values of a zone
/*
  Low is the beginning of the target intensity,
  High is the end of the target intensity.
*/
export enum class Level : bool { Low, High };

export struct HRZone
{
  ZonePair Z1{ 50, 60 };
  ZonePair Z2{ 61, 70 };
  ZonePair Z3{ 71, 80 };
  ZonePair Z4{ 81, 90 };
  ZonePair Z5{ 91, 100 };
} const hrZone;

export constexpr uint8_t minimalHeartRate{ 30 };

export enum PWZ : uint8_t {
  P1 = 1, // 0-54% FTP
  P2 = 2, // 55-75% FTP
  P3 = 3, // 76-90% FTP
  P4 = 4, // 91-105% FTP
  P5 = 5, // 106-120% FTP
  P6 = 6, // 121-150% FTP
  P7 = 7  // >150% FTP
};

export struct PowerZones
{
  ZonePair Z1{ 0, 54 };
  ZonePair Z2{ 55, 75 };
  ZonePair Z3{ 76, 90 };
  ZonePair Z4{ 91, 105 };
  ZonePair Z5{ 106, 120 };
  ZonePair Z6{ 121, 150 };
  ZonePair Z7{ 151, 200 };
} const pwZone;

/*
  Each Interval can be either heart rate or power based. Each of those can be
  either an absolute value, relative to max heart rate or FTP, or it can be a
  target zone. If it is a relative intensity or a target zone, FTP or max heart
  rate should be provided.
*/
export enum class IntensityUnit : uint8_t {
  Watts,
  PercentFTP,
  PowerZone,
  HeartRateBPM,
  PercentMaxHR,
  HeartRateZone
};

export enum class PowerType : uint8_t { Watts, PercentFTP, PowerZone };

export enum class HeartRateUnit : uint8_t {
  HeartRateBPM,
  PercentMaxHR,
  HeartRateZone
};

// low and high target values
export using IntensityPair = std::pair<uint16_t, uint16_t>;
export using HeartRatePair = std::pair<uint8_t, uint8_t>;

// The number at which heart rate intensity enum values begin
export constexpr const uint8_t heartRateOffset{ 3 };

// The number of different intensity types the IntensityUnit enum holds
export constexpr const uint8_t IntensityUnits{ 6 };

// The number of power types
export constexpr const uint8_t powerTypes{ 3 };

export constexpr const uint8_t HeartRateUnits{ 3 };

export using FtpType = uint16_t;
export using HrType = uint8_t;
export using CapacityT = std::variant<FtpType, HrType>; // FTP or max heart
                                                        // rate
export using uintType = uint16_t;
export using voidReturn = std::expected<void, std::string>;
export using uintReturn = std::expected<uintType, std::string>;

template <typename T>
concept IsVoidExpectedC
    = std::is_convertible_v<T, std::expected<void, std::string>>;

template <typename T>
concept IsStringExpectedC
    = std::is_convertible_v<T, std::expected<std::string, std::string>>;

export template <typename T>
concept FileHandlerC = requires (T fileHandler) {
  requires IsVoidExpectedC<decltype (fileHandler.checkFile ())>;
  requires IsStringExpectedC<decltype (fileHandler.getWorkoutName ())>;
  requires IsStringExpectedC<decltype (fileHandler.getWorkoutNotes ())>;
  { fileHandler.getIntervals () };
};

}; // namespace Workouts