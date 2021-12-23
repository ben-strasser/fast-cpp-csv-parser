#include "tinytast.hpp"
#include "csv.h"

#include <fstream>
#include<chrono>

static
const char* CSVM64 = "m64.csv";
const int NUM_1M = 1 << 20;

static
void generate_file()
{
    DESC("generate a csv file about more than 64M");
    std::string line;
    line += ",abcdefg";
    line += ",hijklmn";
    line += ",opq-rst";
    line += ",uvw-xyz";
    line += ",ABCDEFG";
    line += ",HIJKLMN";
    line += ",OPQ-RST";
    line += ",UVW-XYZ";

    std::string head = "id,#1,#2,#3,#4,#5,#6,#7,#8";
    std::ofstream ofs(CSVM64);
    ofs << head << "\n";
    for (int i = 0; i < NUM_1M; ++i)
    {
        ofs << i+1 << line << "\n";
    }
    ofs.close();
}

DEF_TAST(larger_f1, "tast deal large csv file")
{
    DESC("open in csv file about more than 64M");
    std::ifstream ifs(CSVM64);
    if (!ifs.is_open())
    {
        ifs.close();
        generate_file();
        ifs.open(CSVM64);
    }
    COUT_ASSERT(ifs.is_open());

    DESC("read the csv by CSVReader");
    io::CSVReader<9> in(CSVM64, ifs);
    in.read_header(io::ignore_missing_column, "id", "#1", "#2", "#3", "#4", "#5", "#6", "#7", "#8");

    int id;
    std::string c1, c2, c3, c4, c5, c6, c7, c8;
    int count = 0;
    auto start = std::chrono::steady_clock::now();
    while (in.read_row(id, c1, c2, c3, c4, c5, c6, c7, c8))
    {
        // only print if the compare is fail, otherwise print too much
        COUTF(id, ++count);
        COUTF(c1, "abcdefg");
        COUTF(c2, "hijklmn");
        COUTF(c3, "opq-rst");
        COUTF(c4, "uvw-xyz");
        COUTF(c5, "ABCDEFG");
        COUTF(c6, "HIJKLMN");
        COUTF(c7, "OPQ-RST");
        COUTF(c8, "UVW-XYZ");
    }
    auto end = std::chrono::steady_clock::now();
    auto tt = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    DESC("used time in microseconds:");
    COUT(tt.count());
}

DEF_TAST(large_gbk, "tast large gbk string encode convertion")
{
    // "烫" in gbk and utf
    std::string gbk_char {"\xCC\xCC"};
    std::string utf_char{"\xE7\x83\xAB"};

    DESC("construct a long line with 1M gbk chars, in 2M bytes");
    std::string long_line;
    for (int i = 0; i < NUM_1M; ++i)
    {
        long_line.append(gbk_char);
    }
    COUT(long_line.size(), NUM_1M * 2);

    DESC("convert to utf-8, in 3M bytes");
    io::TextEnconv gbk2utf("UTF-8", "GBK");
    COUT(gbk2utf.try_convert(long_line), true);
    COUT(long_line.size(), NUM_1M * 3);
    for (int i = 0; i < NUM_1M; ++i)
    {
        std::string ch = long_line.substr(i * 3, 3);
        COUTF(ch, utf_char);
    }

    DESC("convert back to gbk");
    io::TextEnconv utf2gbk("GBK", "UTF-8");
    COUT(utf2gbk.try_convert(long_line), true);
    COUT(long_line.size(), NUM_1M * 2);
    for (int i = 0; i < NUM_1M; ++i)
    {
        std::string ch = long_line.substr(i * 2, 2);
        COUTF(ch, gbk_char);
    }

    DESC("try to read such long line with LineReader may throw");
    io::LineReader in("string.buf", long_line);
    try
    {
        char* line = in.next_line();
    }
    catch (io::error::base& err)
    {
        COUT(err.what());
    }

    std::string half_line = long_line.substr(NUM_1M);
    COUT(half_line.size(), NUM_1M);
    io::LineReader in2("string.buf", half_line);
    try
    {
        char* line = in2.next_line();
    }
    catch (io::error::base& err)
    {
        COUT(err.what());
    }
}
