#include "tinytast.hpp"
#include "csv.h"

DEF_TAST(linereader_1, "tast LineReader basic use")
{
    std::string source = R"(the first line;
the second line;
the 3rd line;
the 4th line;
)";

    io::LineReader in("string.buf", source.c_str(), source.c_str() + source.size());
    DESC("read line in while loop");
    while (char *line = in.next_line())
    {
        COUT(line);
    }

    io::LineReader in2("string.buf", source);
    std::string line;

    DESC("read line by line");
    line = in2.next_line();
    COUT(line, "the first line;");
    line = in2.next_line();
    COUT(line, "the second line;");
    line = in2.next_line();
    COUT(line, "the 3rd line;");
    line = in2.next_line();
    COUT(line, "the 4th line;");

    char* end = in2.next_line();
    COUT(end == nullptr);
}

DEF_TAST(csvreader_1, "tast csv reader basic use")
{
    std::string source = R"(id,first_name,last_name
1,zhao,yishang
2,qian,erbai
3,sun,sanhui
4,   ,sicui
)";

    DESC("create csv reader with 3 columns");
    io::CSVReader<3> in("string.buf", source);

    DESC("read header");
    in.read_header(io::ignore_missing_column, "id", "first_name", "last_name");
    DESC("test column");
    COUT(in.has_column("id"), true);
    COUT(in.has_column("first_name"), true);
    COUT(in.has_column("last  name"), false);

    DESC("read each row in while loop");
    int id;
    std::string first_name;
    std::string last_name;
    int count = 0;
    while (in.read_row(id, first_name, last_name))
    {
        DESC("read line: %d", ++count);
        COUT(id);
        COUT(first_name);
        COUT(last_name);

        if (id == 2)
        {
            COUT(first_name, "qian");
            COUT(last_name, "erbai");
        }

        if (id == 4)
        {
            COUT(first_name.empty(), true);
            COUT(last_name, "sicui");
        }
    }
    COUT(count, 4);
}

DEF_TAST(csvreader_2, "tast customer csv reader")
{
    std::string source = R"(
# the first line is actually empty, this second line is commented as begin with #
# the csv content with header are following bellow:

#-----|--------------|-------------|
|"id" | "first_name" | "last_name" |
#-----|--------------|-------------|
|"1"  |"zhao"        |"yishang"    |
|"2"  |"qian"        |"erbai"      |
|"3"  |"sun"         |"sanhui"     |

# the body can also has empty or empty line break
# use two "" to get one " in column content
|"4"  |"Li""Lee"     |"sicui"      |
)";

    DESC("test read by LineReader first");
    {
        io::LineReader in("string.buf", source);
        char* line = in.next_line();
        COUT(line != nullptr, true);
        COUT(*line == '\0', true);
        line = in.next_line();
        COUT(line != nullptr, true);
        COUT(*line == '#', true);
    }

    DESC("read by CSVReader with policy");
    io::CSVReader<3,
        io::trim_chars<' ', '\t'>,
        io::double_quote_escape<'|','\"'>,
        io::throw_on_overflow,
        io::single_and_empty_line_comment<'#'>
        > in("string.buf", source);

    in.read_header(io::ignore_extra_column, "id", "first_name", "last_name");

    DESC("read each row in while loop");
    int id;
    std::string first_name;
    std::string last_name;
    int count = 0;
    while (in.read_row(id, first_name, last_name))
    {
        DESC("read line: %d", ++count);
        COUT(id);
        COUT(first_name);
        COUT(last_name);
        if (id == 4)
        {
            COUT(first_name, "Li\"Lee");
        }
    }
    COUT(count, 4);
}

DEF_TAST(csvenconv_1, "tast convert encoding betweet utf8 and gbk")
{
    DESC("The literal string in source code is encoded in utf-8");
    std::string source = R"(id,first_name,last_name,first_name.zh,last_name.zh
1,zhao,yishang,赵,一伤
2,qian,erbai,钱,二败
3,sun,sanhui,孙,三毁
)";

    std::string source_gbk;
    std::string source_utf;
    io::TextEnconv utf2gbk("GBK", "UTF-8");
    io::TextEnconv gbk2utf("UTF-8", "GBK");
    COUT(sizeof(utf2gbk));

    DESC("convert UTF 2 GBK");
    utf2gbk.convert(source, source_gbk);
    COUT(source_gbk.empty(), false);
    DESC("convert GBK back to UTF");
    gbk2utf.convert(source_gbk, source_utf);
    COUT(source_utf.empty(), false);
    COUT(source_utf.size() != source_gbk.size(), true);
    COUT(source_utf == source, true);

    DESC("test create different csv readers from gbk string");

    DESC("read normally not convert encode");
    {
        io::CSVReader<5> in("string.buf", source_gbk);
        in.read_header(io::ignore_missing_column,
                "id", "first_name", "last_name", "first_name.zh", "last_name.zh");
        int id;
        std::string first_name;
        std::string last_name;
        std::string first_name_zh;
        std::string last_name_zh;
        int count = 0;
        while (in.read_row(id, first_name, last_name,first_name_zh, last_name_zh))
        {
            DESC("read line: %d", ++count);
            COUT(id);
            COUT(first_name);
            COUT(last_name);
            // COUT(first_name_zh); // may print messly in utf-8 terminal
            // COUT(last_name_zh);
            if (id == 2)
            {
                COUT(first_name_zh != "钱", true);
                COUT(last_name_zh != "二败, true");
            }
        }
    }

    DESC("convert encode for some columns: solution 1");
    {
        io::CSVReader<5> in("string.buf", source_gbk);
        in.read_header(io::ignore_missing_column,
                "id", "first_name", "last_name", "first_name.zh", "last_name.zh");
        int id;
        std::string first_name;
        std::string last_name;
        std::string first_name_zh;
        std::string last_name_zh;
        DESC("use TextEnconv to convert column string value after read_row");
        int count = 0;
        while (in.read_row(id, first_name, last_name,first_name_zh, last_name_zh))
        {
            DESC("read line: %d", ++count);
            gbk2utf.try_convert(first_name_zh);
            gbk2utf.try_convert(last_name_zh);
            COUT(id);
            COUT(first_name);
            COUT(last_name);
            COUT(first_name_zh);
            COUT(last_name_zh);
            if (id == 2)
            {
                COUT(first_name_zh == "钱", true);
                COUT(last_name_zh == "二败", true);
            }
        }
    }

    DESC("convert encode for some columns: solution 2 (prefered)");
    {
        io::CSVReader<5> in("string.buf", source_gbk);
        in.read_header(io::ignore_missing_column,
                "id", "first_name", "last_name", "first_name.zh", "last_name.zh");
        int id;
        std::string first_name;
        std::string last_name;
        std::string first_name_zh;
        std::string last_name_zh;
        int count = 0;
        DESC("use TieString in read_row while loop");
        while (in.read_row(id, first_name, last_name,
                    io::TieString(gbk2utf, first_name_zh),
                    io::TieString(gbk2utf, last_name_zh)
                    ))
        {
            DESC("read line: %d", ++count);
            COUT(id);
            COUT(first_name);
            COUT(last_name);
            COUT(first_name_zh);
            COUT(last_name_zh);
            if (id == 2)
            {
                COUT(first_name_zh, "钱");
                COUT(last_name_zh, "二败");
            }
        }
    }

    DESC("convert encode for some columns: solution 3");
    {
        io::CSVReader<5> in("string.buf", source_gbk);
        in.read_header(io::ignore_missing_column,
                "id", "first_name", "last_name", "first_name.zh", "last_name.zh");
        int id;
        std::string first_name;
        std::string last_name;
        std::string first_name_zh;
        std::string last_name_zh;

        DESC("declare TieString object before read_row while loop");
        io::TieString temp_first_name_zh(gbk2utf, first_name_zh);
        io::TieString temp_last_name_zh(gbk2utf, last_name_zh);
        DESC("but make sure not use TieString object any more after the tied string and covertor beyond scope!");
        COUT(sizeof(temp_first_name_zh));
        COUT(sizeof(io::TieString));

        int count = 0;
        while (in.read_row(id, first_name, last_name, temp_first_name_zh, temp_last_name_zh))
        {
            DESC("read line: %d", ++count);
            COUT(id);
            COUT(first_name);
            COUT(last_name);
            COUT(first_name_zh);
            COUT(last_name_zh);
            if (id == 2)
            {
                COUT(first_name_zh, "钱");
                COUT(last_name_zh, "二败");
            }
        }
    }
}
