export module intensity;

import common;
import config;
import std;
import std.compat;

namespace Workouts
{

export class Intensity
{
public:
  Intensity () = default;

  explicit Intensity (uint16_t intensity, IntensityUnit unit,
                      uint16_t capacity, Level level = Level::Low) noexcept
      : m_target (IntensityPair{ intensity, intensity }), m_unit (unit),
        m_level (level), m_capacity (capacity)
  {
    if (unit == IntensityUnit::Watts || unit == IntensityUnit::PercentFTP
        || unit == IntensityUnit::PowerZone)
      {
        m_capacity = capacity;
      }
    else
      {
        m_capacity = static_cast<HrType> (capacity);
      }
  }

  void setTarget (uint16_t target, IntensityUnit unit,
                  Level level = Level::Low) noexcept
  {
    m_unit = unit;
    level == Level::Low ? m_target.first = target : m_target.second = target;
  }

  void setTarget (IntensityPair target) noexcept { m_target = target; }

  constexpr uint16_t getTarget (Level level = Level::Low) const noexcept
  { return level == Level::Low ? m_target.first : m_target.second; }

  constexpr std::string getUnitStr () const noexcept
  {
    std::array<std::string, IntensityUnits> units{
      "watts",          "\%FTP", "power zone", "bpm", "\%max heart rate",
      "heart rate zone"
    };
    return units.at (std::to_underlying (m_unit));
  }

  constexpr IntensityUnit getType () const noexcept { return m_unit; }

  void setFTP (uint16_t ftp) noexcept { m_capacity = ftp; }
  void setMaxHeartRate (uint8_t maxHeartRate) noexcept
  { m_capacity = maxHeartRate; }

  [[deprecated ("Not yet implemented")]] static constexpr uint16_t
  getWatts (Level level = Level::Low) noexcept
  { return 0; };
  [[deprecated ("Not yet implemented")]] static constexpr uint16_t
  getPercentFTP (Level level = Level::Low) noexcept
  { return 0; };
  [[deprecated ("Not yet implemented")]] static constexpr uint16_t
  getPowerZone (Level level = Level::Low) noexcept
  { return 0; };
  [[deprecated ("Not yet implemented")]] static constexpr uint16_t
  getHeartRateBPM (Level level = Level::Low) noexcept
  { return 0; };
  [[deprecated ("Not yet implemented")]] static constexpr uint16_t
  getPercentMaxHR (Level level = Level::Low) noexcept
  { return 0; };
  [[deprecated ("Not yet implemented")]] static constexpr uint16_t
  getHeartRateZone (Level level = Level::Low) noexcept
  { return 0; };

private:
  /**
   * @brief Converts an absolute power or heart rate value to a
   * percentage of ftp or max heart rate
   *
   * @param intensity Intensity in watts or bpm
   * @param capacityValue FTP or max heart rate
   * @return constexpr std::expected<uint16_t, std::string>
   */
  static constexpr std::expected<uint16_t, std::string>
  convertToRelative (uint16_t intensity, uint16_t capacityValue) noexcept
  {
    if (capacityValue == 0)
      {
        return std::unexpected ("Please provide a valid ftp or maxHeartRate "
                                "first before setting power or heartrate");
      }
    constexpr uint16_t percent{ 100 };
    intensity *= percent;
    return intensity / capacityValue;
  }

  /**
   * @brief Converts a relative value like ftp or max heart rate back to
   * an absolute power or heart rate value in watts or bpm
   *
   * @param intensity The relative intensity
   * @param value FTP or max heart rate
   * @return constexpr uint16_t
   */
  static constexpr uint16_t convertToAbsolute (uint16_t intensity,
                                               uint16_t value) noexcept
  {
    constexpr double percent{ 100.0 };
    // Do the divsion and multiplication as double and cast the result
    // back to uint16_t
    return static_cast<uint16_t> (static_cast<double> (intensity)
                                  * static_cast<double> (value) / percent);
  }

  static constexpr uint8_t convertToPowerZone (uint16_t intensity,
                                               uint16_t ftp = 0) noexcept
  {
    if (ftp > 0)
      // Intensity is % of FTP
      // Calculate relative power first
      {
        /*         if (auto retVal{ convertToRelative (intensity, ftp) };
           retVal)
                  {
                    intensity = *retVal;
                  } */
      }

    if (intensity <= pwZone.Z1.second)
      {
        return PWZ::P1;
      }
    if (intensity <= pwZone.Z2.second)
      {
        return PWZ::P2;
      }
    if (intensity <= pwZone.Z3.second)
      {
        return PWZ::P3;
      }
    if (intensity <= pwZone.Z4.second)
      {
        return PWZ::P4;
      }
    if (intensity <= pwZone.Z5.second)
      {
        return PWZ::P5;
      }
    if (intensity <= pwZone.Z6.second)
      {
        return PWZ::P6;
      }
    return PWZ::P7;
  }

  static constexpr uint8_t convertFromPowerZone (PWZ zone,
                                                 bool getLower) noexcept
  {
    switch (zone)
      {
      case PWZ::P1: return getLower ? pwZone.Z1.first : pwZone.Z1.second;
      case PWZ::P2: return getLower ? pwZone.Z2.first : pwZone.Z2.second;
      case PWZ::P3: return getLower ? pwZone.Z3.first : pwZone.Z3.second;
      case PWZ::P4: return getLower ? pwZone.Z4.first : pwZone.Z4.second;
      case PWZ::P5: return getLower ? pwZone.Z5.first : pwZone.Z5.second;
      case PWZ::P6: return getLower ? pwZone.Z6.first : pwZone.Z6.second;
      case PWZ::P7: return getLower ? pwZone.Z7.first : pwZone.Z7.second;
      default: std::unreachable;
      }
  }

  static constexpr uint8_t
  convertToHeartRateZone (uint8_t intensity, uint8_t maxHeartRate = 0) noexcept
  {
    if (maxHeartRate > 0)
      {
        /*         if (auto retVal{ convertToRelative (intensity, maxHeartRate)
           }; retVal)
                  {
                    intensity = *retVal;
                  } */
      }

    if (intensity > hrZone.Z1.first && intensity <= hrZone.Z1.second)
      {
        return HRZ::H1;
      }
    if (intensity > hrZone.Z2.first && intensity <= hrZone.Z2.second)
      {
        return HRZ::H2;
      }
    if (intensity > hrZone.Z3.first && intensity <= hrZone.Z3.second)
      {
        return HRZ::H3;
      }
    if (intensity > hrZone.Z4.first && intensity <= hrZone.Z4.second)
      {
        return HRZ::H4;
      }
    if (intensity > hrZone.Z5.first && intensity <= hrZone.Z5.second)
      {
        return HRZ::H5;
      }
    return 0;
  }

  static constexpr uint8_t
  convertFromHeartRateZone (HRZ intensity, bool getLower = true) noexcept
  {
    switch (intensity)
      {
      case HRZ::H1: return getLower ? hrZone.Z1.first : hrZone.Z1.second;
      case HRZ::H2: return getLower ? hrZone.Z2.first : hrZone.Z2.second;
      case HRZ::H3: return getLower ? hrZone.Z3.first : hrZone.Z3.second;
      case HRZ::H4: return getLower ? hrZone.Z4.first : hrZone.Z4.second;
      case HRZ::H5: return getLower ? hrZone.Z5.first : hrZone.Z5.second;
      default: std::unreachable;
      }
  }

private:
  IntensityPair m_target{ 0, 0 };
  IntensityUnit m_unit{ IntensityUnit::Watts };
  Level m_level{ Level::Low };
  CapacityT m_capacity;
};

}; // namespace Workouts