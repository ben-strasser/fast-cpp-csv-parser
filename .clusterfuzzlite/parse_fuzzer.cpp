
#include "csv.h"
#include <unistd.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  char filename[256];
  sprintf(filename, "/tmp/libfuzzer.%d.csv", getpid());
  FILE *fp = fopen(filename, "wb");
  if (!fp)
    return 0;
  fwrite(data, size, 1, fp);
  fclose(fp);

  io::CSVReader<3> in(filename);
  try {
    in.read_header(io::ignore_extra_column, "vendor", "col2", "col3");
    std::string vendor;
    int col2;
    double col3;
    while (in.read_row(vendor, col2, col3)) {
    }
  } catch (...) {
  }

  unlink(filename);
  return 0;
}