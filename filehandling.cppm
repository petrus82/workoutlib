export module filehandling;

import config;
import common;
import std;
import std.compat;

namespace Workouts
{
export enum class FileType : uint8_t { Fit, Plan, Erg, Mrc };
/*
  Internal free functions and declarations to handle ERG and MRC files.
*/
export struct TextFileFormat
{
  std::string_view headerStart; // Starting sequence
  std::string_view headerSpec;  // Extra sequence, like unit specifications
  std::string_view nameTag;     // Workout name sequence
  std::string_view
      headerDuration;       // Optional duration tag which specifies the total
                            // workout duration, required for plan files
  std::string_view noteTag; // Note sequence
  std::string_view intensityUnitTag;  // Intensity Unit specification
  std::string_view headerSeparator;   // Separates key from values (e.g. '=')
  std::string_view headerEnd;         // Header closing sequence
  std::string_view intervalTag;       // Interval preceding sequence
  std::string_view intervalSeparator; // Separates interval keys from their
                                      // values (e.g. '=' or ':')
  std::string_view subIntervalTag;
  std::string_view repeatTag;
  std::string_view intervalIntensityAbsLoTag; // Intensity specification
  std::string_view intervalIntensityAbsHiTag;
  std::string_view intervalIntensityRelLoTag;
  std::string_view intervalIntensityRelHiTag;
  std::string_view intervalDurationTag; // Duration specification
  IntensityUnit type;
};

export constexpr std::expected<std::string, std::string>
readFileContent (const std::filesystem::path &file)
{
  std::ifstream filestream (file);
  if (filestream)
    {
      // Get file size and reserve memory
      filestream.seekg (0, std::ios::end);

      // std::ifstream::read does not take more than std::streamsize for
      // the file size
      auto fileSize
          = static_cast<std::streamsize> (std::filesystem::file_size (file));
      std::string content (fileSize, '\0');

      // Read file into string
      filestream.seekg (0, std::ios::beg);
      filestream.read (content.data (), fileSize);
      return content;
    }
  return std::unexpected ("Cannot open file.");
}

export constexpr auto processContent (std::string_view fileContent,
                                      TextFileFormat format)
{
  auto intervalPos = fileContent.find (format.intervalTag);
  std::string_view workout{ fileContent.substr (0, intervalPos) };
  if (workout.starts_with (format.headerStart))
    {
      workout.remove_prefix (format.headerStart.length ());
    }
  intervalPos += format.intervalTag.length ();
  std::string_view intervals{ fileContent.substr (
      intervalPos, fileContent.length () - intervalPos) };
  return std::pair{ workout, intervals };
}

export constexpr std::expected<std::ifstream, std::string>
getFileStream (const std::filesystem::path &file) noexcept
{
  std::ifstream filestream (file, std::ios::binary);
  if (!filestream)
    {
      return std::unexpected (std::format ("Cannot open file: {}/{}",
                                           file.parent_path ().string (),
                                           file.string ()));
    }
  return filestream;
}
};