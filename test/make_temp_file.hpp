#ifndef MAC_TIME_TRACKER_MAKE_TEMP_FILE_HPP
#define MAC_TIME_TRACKER_MAKE_TEMP_FILE_HPP

#include <fstream>
#include <stdexcept>
#include <string>

#include <stdio.h> // for popen(), pclose()

// makes temp file with contents
static inline std::string makeTempFile(const std::string &contents = "") {
  // makes empty temp file
  char filename[256];
  {
    FILE *const fp = popen("mktemp", "r");
    if (!fp) {
      throw std::runtime_error("makeTempFile(): popen");
    }

    if (fscanf(fp, "%s", filename) != 1) {
      throw std::runtime_error("makeTempFile(): fscanf");
    }

    pclose(fp);
  }

  // write contents to the temp file
  if (!contents.empty()) {
    std::ofstream ofs(filename);
    if (!ofs) {
      throw std::runtime_error("makeTempFile(): opening std::ofstream");
    }

    ofs << contents;
    if (!ofs) {
      throw std::runtime_error("makeTempFile(): writing to std::ofstream");
    }
  }

  return filename;
}

#endif