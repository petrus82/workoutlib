module;
#include "sha2.h"

export module sha256;
import std;

export std::string sha256sum (const std::filesystem::path &file)
{
  std::string buf (SHA256_DIGEST_STRING_LENGTH, '\0');
  SHA256File (file.c_str (), buf.data ());
  // Remove final \0
  buf.pop_back ();
  return buf;
}