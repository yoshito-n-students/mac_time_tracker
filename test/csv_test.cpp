#include <fstream>
#include <stdexcept>
#include <string>

#include <stdio.h> // for popen(), pclose()

#include <gtest/gtest.h>

#include <mac_time_tracker/csv.hpp>

namespace mtt = mac_time_tracker;

// makes temp file with contents
std::string makeTempFile(const std::string &contents = "") {
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

TEST(CSV, fromFile) {
  // make csv file in system temp directory
  const std::string temp_file =
      makeTempFile(R"(Field 0,Field 1,Field 2)" // L0
                   "\n"
                   R"(Field 0,"Field 1, with comma",Field 2)" // L1
                   "\n"
                   R"(Field 0,Field 1 with \"embedded quote\",Field 2)" // L2
                   "\n"
                   R"(Field 0,Field 1 with \n new line,Field 2)" // L3
                   "\n"
                   R"(Field 0,Field 1 with embedded \\,Field 2 with two embedded \\\\)" // L4
                   "\n"
                   R"(Field 0,"Complex field 1\nwith \",\" and \"\\\"",Field 2)" // L5
                   "\n"
                   "" // L6 (empty)
                   "\n"
                   R"(,Field 1 following empty field 0,Field 2)" // L7
                   "\n"
                   R"( ,Field 1 following a whitespace on field 0,Field 2)" // L8
                   "\n"
                   R"(Field 0,Field 1 followed empty field 2,)" // L9
                   "\n"
                   R"(Field 0,Field 1 followed a whitespace on field 2, )" // L10
      );
  // read the file
  const mtt::CSV csv = mtt::CSV::fromFile(temp_file);
  // number of lines
  ASSERT_EQ(11, csv.size());
  // L0
  ASSERT_EQ(3, csv[0].size());
  ASSERT_STREQ("Field 0", csv[0][0].c_str());
  ASSERT_STREQ("Field 1", csv[0][1].c_str());
  ASSERT_STREQ("Field 2", csv[0][2].c_str());
  // L1
  ASSERT_EQ(3, csv[1].size());
  ASSERT_STREQ("Field 1, with comma", csv[1][1].c_str());
  // L2
  ASSERT_EQ(3, csv[2].size());
  ASSERT_STREQ(R"(Field 1 with "embedded quote")", csv[2][1].c_str());
  // L3
  ASSERT_EQ(3, csv[3].size());
  ASSERT_STREQ("Field 1 with \n new line", csv[3][1].c_str());
  // L4
  ASSERT_EQ(3, csv[4].size());
  ASSERT_STREQ(R"(Field 1 with embedded \)", csv[4][1].c_str());
  ASSERT_STREQ(R"(Field 2 with two embedded \\)", csv[4][2].c_str());
  // L5
  ASSERT_EQ(3, csv[5].size());
  ASSERT_STREQ("Complex field 1"
               "\n"
               R"(with "," and "\")",
               csv[5][1].c_str());
  // L6
  ASSERT_EQ(0, csv[6].size());
  // L7
  ASSERT_EQ(3, csv[7].size());
  ASSERT_STREQ("", csv[7][0].c_str());
  // L8
  ASSERT_EQ(3, csv[8].size());
  ASSERT_STREQ(" ", csv[8][0].c_str());
  // L9
  ASSERT_EQ(3, csv[9].size());
  ASSERT_STREQ("", csv[9][2].c_str());
  // L10
  ASSERT_EQ(3, csv[10].size());
  ASSERT_STREQ(" ", csv[10][2].c_str());
  // Failure cases
  ASSERT_THROW(mtt::CSV::fromFile("/dir_that_does_not_exist/" + temp_file), std::runtime_error);
  ASSERT_THROW(mtt::CSV::fromFile(temp_file + "_suffix_that_makes_filename_invalid"),
               std::runtime_error);
}

TEST(CSV, toFile) {
  // make 2 temp files, one is a csv as source, another is empty as destination
  const std::string temp_src_file =
      makeTempFile(R"(Field,,Field between empty fields,)" // L0
                   "\n"
                   R"("Field, with comma",Field with \"embedded quote\")" // L1
                   "\n"
                   R"(Field with \n new line,Field with embedded \\)" // L2
                   "\n"
                   R"("Complex field\nwith \"\\\\\" and \",,\"")" // L3
      );
  const std::string temp_dst_file = makeTempFile();
  // read from the source file, write to and read from the destination file
  const mtt::CSV src_csv = mtt::CSV::fromFile(temp_src_file);
  src_csv.toFile(temp_dst_file);
  const mtt::CSV dst_csv = mtt::CSV::fromFile(temp_dst_file);
  // compare data from the source and destination files
  ASSERT_EQ(4, dst_csv.size());    // number of lines
  ASSERT_EQ(4, dst_csv[0].size()); // L0
  ASSERT_EQ(2, dst_csv[1].size()); // L1
  ASSERT_EQ(2, dst_csv[2].size()); // L2
  ASSERT_EQ(1, dst_csv[3].size()); // L3
  ASSERT_EQ(src_csv, dst_csv);     // all
}