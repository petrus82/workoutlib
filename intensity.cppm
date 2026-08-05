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

  /**
   * @brief Constructs an Intensity object.
   *
   * @param intensity The uint16_t intensity value, can be either power or
   * heart rate.
   * @param unit The unit of intensity (Watts, %FTP, PowerZone, HeartRateBPM,
   * %MaxHR or HeartRateZone).
   * @param capacity The capacity value (either the FTP in watts or MaxHR in
   * BPM).
   * @param level Defaults to Level::Low, can be set to Level::High to indicate
   * what intensity level was meant.
   */
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

  explicit Intensity (PWZ zone, uint16_t capacity)
      : m_target (IntensityPair{ zone, zone }),
        m_unit (IntensityUnit::PowerZone), m_capacity (capacity)
  {
  }

  explicit Intensity (IntensityPair intensity, IntensityUnit unit,
                      uint16_t capacity) noexcept
      : m_target (intensity), m_unit (unit), m_capacity (capacity)
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

  /**
   * @brief Sets a specific target intensity value with a given unit and level.
   *
   * @param target The absolute intensity value or a pair for relative
   * intensity.
   * @param unit The unit for the target intensity.
   * @param level The level for the target intensity, defaults to Level::Low.
   */
  void setTarget (uint16_t target, IntensityUnit unit,
                  Level level = Level::Low) noexcept
  {
    m_unit = unit;
    level == Level::Low ? m_target.first = target : m_target.second = target;
  }

  /**
   * @brief Sets a specific target intensity value using an intensity pair.
   *
   * @param target The IntensityPair containing the relative intensity values.
   */
  void setTarget (IntensityPair target) noexcept { m_target = target; }

  /**
   * @brief Retrieves the target intensity value based on the specified level.
   *
   * @param level The level to retrieve the target intensity from (Level::Low
   * or Level::High).
   * @return The target intensity value.
   */
  constexpr uint16_t getTarget (Level level = Level::Low) const noexcept
  { return level == Level::Low ? m_target.first : m_target.second; }

  /**
   * @brief Gets the string representation of the current intensity unit.
   *
   * @return std::string The string name of the current intensity unit.
   */
  constexpr std::string getUnitStr () const noexcept
  {
    std::array<std::string, IntensityUnits> units{
      "watts",          "\%FTP", "power zone", "bpm", "\%max heart rate",
      "heart rate zone"
    };
    return units.at (std::to_underlying (m_unit));
  }

  /**
   * @brief Gets the current intensity unit.
   *
   * @return IntensityUnit The current unit of intensity.
   */
  constexpr IntensityUnit getType () const noexcept { return m_unit; }

  /**
   * @brief Sets the FTP value for calculating relative intensity or power
   * zones from absolute values or vice versa.
   *
   * @param ftp The FTP value in watts.
   */
  void setFTP (uint16_t ftp) noexcept { m_capacity = ftp; }

  /**
   * @brief Sets the maximum heart rate value for calculating relative
   * intensity or heart rate zones from absolute values or vice versa.
   *
   * @param maxHeartRate The maximum heart rate value in bpm.
   */
  void setMaxHeartRate (uint8_t maxHeartRate) noexcept
  { m_capacity = maxHeartRate; }

  /**
   * @brief Returns the absolute wattage value for a given level. If Intensity
   * was constructed with a relative value, it will be converted to an absolute
   * value using the provided FTP.
   */
  constexpr uint16_t getWatts (Level level = Level::Low) noexcept
  {
    auto intensity{ getTarget (level) };
    if (m_unit == IntensityUnit::Watts)
      // no conversion needed
      {
        return intensity;
      }
    else if (m_unit == IntensityUnit::PercentFTP)
      {
        return convertToAbsolute (intensity, std::get<FtpType> (m_capacity));
      }
    else if (m_unit == IntensityUnit::PowerZone)
      {
        return static_cast<uint16_t> (convertToAbsolute (
            convertFromPowerZone (static_cast<PWZ> (intensity),
                                  level == Level::Low),
            std::get<FtpType> (m_capacity)));
      }
  };

  /**
   * @brief Returns the %FTP value for a given level.
   * Ensures a valid FTP is provided before performing the conversion. If the
   * FTP is not set, it returns an unexpected value with an error message.
   */
  constexpr std::expected<uint16_t, std::string>
  getPercentFTP (Level level = Level::Low) noexcept
  {
    auto intensity{ getTarget (level) };
    if (m_unit == IntensityUnit::PercentFTP)
      {
        return intensity;
      }
    else if (m_unit == IntensityUnit::Watts)
      {
        return convertToRelative (intensity, std::get<FtpType> (m_capacity));
      }
    else if (m_unit == IntensityUnit::PowerZone)
      {
        return convertFromPowerZone (static_cast<PWZ> (intensity),
                                     level == Level::Low);
      }
  };

  /**
   * @brief Returns the Power Zone value for a given level.
   */
  constexpr uint16_t getPowerZone (Level level = Level::Low) noexcept
  {
    if (m_unit == IntensityUnit::PowerZone)
      {
        return getTarget (level);
      }
    return convertToPowerZone (getTarget (level),
                               std::get<FtpType> (m_capacity), m_unit);
  };

  /**
   * @brief Returns the Heart Rate BPM value for a given level.
   * @deprecated Not yet implemented
   */
  [[deprecated ("Not yet implemented")]] static constexpr uint16_t
  getHeartRateBPM (Level level = Level::Low) noexcept
  { return 0; };

  /**
   * @brief Returns the %MaxHR value for a given level.
   * @deprecated Not yet implemented
   */
  [[deprecated ("Not yet implemented")]] static constexpr uint16_t
  getPercentMaxHR (Level level = Level::Low) noexcept
  { return 0; };

  /**
   * @brief Returns the Heart Rate Zone value for a given level.
   * @deprecated Not yet implemented
   */
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

  /**
   * @brief Converts an absolute intensity value to a power zone based on the
   * provided FTP.
   *
   * @param intensity The absolute intensity value in watts.
   * @param ftp The FTP value in watts (default is 0).
   * @return constexpr uint8_t The corresponding power zone (PWZ).
   */
  static constexpr uint8_t convertToPowerZone (uint16_t intensity,
                                               uint16_t ftp,
                                               IntensityUnit unit) noexcept
  {
    if (unit == IntensityUnit::PowerZone)
      {
        return intensity;
      }

    if (ftp > 0 && unit == IntensityUnit::Watts)
      // Calculate relative power first
      {
        if (auto retVal{ convertToRelative (intensity, ftp) }; retVal)
          {
            intensity = *retVal;
          }
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

  /**
   * @brief Converts a power zone to an absolute intensity value based on the
   * provided FTP. This is invoked by calling getIntensity with a
   *
   * @param zone The power zone (PWZ).
   * @param getLower If true, returns the lower bound of the zone; otherwise,
   * returns the upper bound.
   * @return constexpr uint8_t The corresponding absolute intensity value in
   * watts.
   */
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

  /**
   * @brief Converts an absolute heart rate value to a heart rate zone based on
   * the provided maximum heart rate.
   *
   * @param intensity The absolute heart rate value in bpm.
   * @param maxHeartRate The maximum heart rate value in bpm (default is 0).
   * @return constexpr uint8_t The corresponding heart rate zone (HRZ).
   */
  static constexpr uint8_t
  convertToHeartRateZone (uint8_t intensity, uint8_t maxHeartRate = 0) noexcept
  {
    if (maxHeartRate > 0)
      {
        if (auto retVal{ convertToRelative (intensity, maxHeartRate) }; retVal)
          {
            intensity = *retVal;
          }
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

  /**
   * @brief Converts a heart rate zone to an absolute heart rate value based on
   * the provided maximum heart rate.
   *
   * @param zone The heart rate zone (HRZ).
   * @param getLower If true, returns the lower bound of the zone; otherwise,
   * returns the upper bound.
   * @return constexpr uint8_t The corresponding absolute heart rate value in
   * bpm.
   */
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