/** 
 * @file tinytast.hpp
 * @author lymslive 
 * @date 2021-12-23
 * @brief a simple lightweight header only unit test framework
 * @license BSD-3
 * */
#ifndef TINYTAST_HPP__
#define TINYTAST_HPP__

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

namespace tast
{

template<typename T>
class Singleton
{
public:
    static T* GetInstance()
    {
        static T instance;
        return &instance;
    }

protected:
    Singleton() {}
private:
    Singleton(const T&);
    void operator= (const T&);
};

const int MAX_PRINT_BUFFER = 1024;
inline
void Print(const char* format, ...)
{
    static char buffer[MAX_PRINT_BUFFER] = {0};

    va_list vlist;
    va_start(vlist, format);
    vsnprintf(buffer, sizeof(buffer), format, vlist);
    va_end(vlist);

    printf("%s\n", buffer);
}

class CTinyCli
{
protected:
    std::map<std::string, std::string> m_mapOption;
    std::vector<std::string> m_vecArg;

    void ParseCli(int argc, char** argv)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (!argv[i] || argv[i][0] == '\0')
            {
                continue;
            }

            // prefix --
            const char* k = argv[i];
            while (*k == '-')
            {
                ++k;
            }

            // midfix =
            const char* v = NULL;
            for (const char* s = k; *s != '\0'; ++s)
            {
                if (*s == '=')
                {
                    v = ++s;
                    break;
                }
            }

            if (v && *v != '\0')
            {
                // key=val --key==val
                std::string val = v;
                std::string key(k, v-1);
                m_mapOption[key] = val;
            }
            else if (k != argv[i] && *k != '\0')
            {
                // --key
                std::string key(k);
                m_mapOption[key] = "1";
            }
            else
            {
                // normal arg or only --
                m_vecArg.push_back(argv[i]);
            }
        }
    }

    void ParseCli(const std::vector<std::string>& args)
    {
        if (args.empty())
        {
            return;
        }
        std::vector<const char*> argv;
        argv.push_back(""); // argv[0] as program name
        for (size_t i = 0; i < args.size(); ++i)
        {
            argv.push_back(const_cast<char*>(args[i].c_str()));
        }
        return ParseCli((int)argv.size(), const_cast<char**>(&argv[0]));
    }
};

// raw  tast function pointer
typedef void (*PFTAST)();

// tast class
class CTastCase
{
public:
    CTastCase(const std::string& desc) : m_description(desc) {}
    virtual ~CTastCase() {}
    virtual void run() = 0;
    const std::string& Description() const
    { return m_description; }
private:
    std::string m_description;
};

class CTastMgr : public Singleton<CTastMgr>, public CTinyCli
{
private:
    std::map<std::string, CTastCase*> m_mapTastCase;
    std::vector<std::string> m_listFail;
    int m_currentFail;
    int m_passedCase;
    int m_failedCase;
    bool m_coutFail;   // only cout failed statement, ignore passed statement
    bool m_coutSilent; // not cout extra verbose information

public:
    bool CoutFail() const { return m_coutFail || m_coutSilent; }
    bool CoutSilent() const { return m_coutSilent; }

public:
    CTastMgr() : m_currentFail(0), m_passedCase(0), m_failedCase(0), m_coutFail(false), m_coutSilent(false) {}
    ~CTastMgr()
    {
        for (std::map<std::string, CTastCase*>::iterator it = m_mapTastCase.begin(); it != m_mapTastCase.end(); ++it)
        {
            if (it->second)
            {
                delete it->second;
                it->second = NULL;
            }
        }
    }

    void AddTast(const std::string& name, CTastCase* instance)
    {
        if (!instance)
        {
            return;
        }
        if (m_mapTastCase.find(name) == m_mapTastCase.end())
        {
            m_mapTastCase[name] = instance;
        }
    }

    void AddFail()
    {
        m_currentFail++;
    }

    static const char* pass_case(bool tf)
    {
        return tf ? "PASS" : "FAIL";
    }

    void PreRunTast(const std::string& name)
    {
        m_currentFail = 0;
        Print("## run %s()", name.c_str());
    }

    void PostRunTast(const std::string& name)
    {
        if (m_currentFail == 0)
        {
            m_passedCase++;
        }
        else
        {
            m_failedCase++;
            m_listFail.push_back(name);
        }

        if (!m_coutSilent)
        {
            if (m_currentFail == 0)
            {
                Print("<< [%s] %s", pass_case(true), name.c_str());
            }
            else
            {
                Print("<< [%s] %d @ %s", pass_case(false), m_currentFail, name.c_str());
            }
            Print("");
        }
    }

    void RunTast(const std::string& name, PFTAST fun)
    {
        PreRunTast(name);
        fun();
        PostRunTast(name);
    }

    void RunTast(const std::string& name, CTastCase* pTastCase)
    {
        if (!pTastCase) { return; }

        PreRunTast(name);
        pTastCase->run();
        PostRunTast(name);
    }

    void RunTast(const std::string& name)
    {
        if (name.empty()) { return; }

        std::map<std::string, CTastCase*>::iterator it = m_mapTastCase.find(name);
        if (it != m_mapTastCase.end())
        {
            return RunTast(name, it->second);
        }
        else if (name.size() == 1)
        {
            return RunTast(name, '^');
        }
        else
        {
            size_t iend = name.size() - 1;
            if (name[0] == '^')
            {
                return RunTast(name.substr(1), '^');
            }
            else if (name[iend] == '*')
            {
                return RunTast(name.substr(0, iend), '^');
            }
            else if (name[iend] == '$')
            {
                return RunTast(name.substr(0, iend), '$');
            }
            else if (name[0] == '*')
            {
                return RunTast(name.substr(1), '$');
            }
            else
            {
                return RunTast(name, '%');
            }
        }
    }

    void RunTast(const std::string& arg, char filter)
    {
        for (std::map<std::string, CTastCase*>::iterator it = m_mapTastCase.begin(); it != m_mapTastCase.end(); ++it)
        {
            const std::string& name = it->first;
            if (arg.size() > name.size())
            {
                continue;
            }

            size_t pos = name.find(arg);
            if (filter == '%' && pos != std::string::npos)
            {
                RunTast(name, it->second);
            }
            else if (filter == '^' && pos == 0)
            {
                RunTast(name, it->second);
            }
            else if (filter == '$' && pos == name.size() - arg.size())
            {
                RunTast(name, it->second);
            }
        }
    }

    int RunTast()
    {
        if (!m_vecArg.empty())
        {
            for (size_t i = 0; i < m_vecArg.size(); ++i)
            {
                RunTast(m_vecArg[i]);
            }
        }
        else
        {
            for (std::map<std::string, CTastCase*>::iterator it = m_mapTastCase.begin(); it != m_mapTastCase.end(); ++it)
            {
                RunTast(it->first, it->second);
            }
        }
        return Summary();
    }

    int RunTast(int argc, char** argv)
    {
        ParseCli(argc, argv);
        if (m_mapOption.find("list") != m_mapOption.end())
        {
            return ListCase(false);
        }
        else if (m_mapOption.find("List") != m_mapOption.end())
        {
            return ListCase(true);
        }
        else if (m_mapOption.find("help") != m_mapOption.end())
        {
            return ListHelp();
        }
        else if (m_mapOption.find("cout") != m_mapOption.end())
        {
            std::string& arg = m_mapOption["cout"];
            if (arg == "fail")
            {
                m_coutFail = true;
            }
            else if (arg == "silent")
            {
                m_coutSilent = true;
            }
        }
        return RunTast();
    }

    int ListCase(bool bWithDesc)
    {
        for (std::map<std::string, CTastCase*>::iterator it = m_mapTastCase.begin(); it != m_mapTastCase.end(); ++it)
        {
            if (bWithDesc && it->second)
            {
                Print("%s:\t%s", it->first.c_str(), it->second->Description().c_str());
            }
            else
            {
                Print("%s", it->first.c_str());
            }
        }
        return 0;
    }

    int ListHelp()
    {
        Print("./tast_program [options] [tastcase ...]");
        Print("  --list (--List): list all tastcase (also with description)");
        Print("  --cout=[fail|silent]: print only failed statement");
        Print("  --help: print this message");
        return 0;
    }

    int Summary()
    {
        Print("## Summary");
        Print("<< [%s] %d", pass_case(true), m_passedCase);
        Print("<< [%s] %d", pass_case(false), m_failedCase);
        for (size_t i = 0; i < m_listFail.size(); ++i)
        {
            Print("!! %s", m_listFail[i].c_str());
        }
        return m_failedCase;
    }
};

template <typename T>
class CTastBuilder
{
public:
    CTastBuilder(const std::string& name, const std::string& desc = "")
    {
        CTastMgr::GetInstance()->AddTast(name, new T(desc));
    }
};

struct SLocation
{
    const char* file;
    int line;
    const char* function;

    SLocation(const char* pFile, int iLine, const char* pFunction)
        : file(pFile), line(iLine), function(pFunction)
    {}
};

class CStatement
{
private:
    SLocation m_stLocation;
    const char* m_pExpression;
    bool m_bCoutFailOnly;

public:
    CStatement(const SLocation& stLocation, const char* pExpression, bool bCoutFailOnly = false)
        : m_stLocation(stLocation), m_pExpression(pExpression), m_bCoutFailOnly(bCoutFailOnly)
    {}

    static const char* pass_cout(bool tf)
    {
        return tf ? "OK" : "NO";
    }

    std::string cout_value(bool tf)
    {
        return tf ? "ture" : "false";
    }

    template <typename T>
    std::string cout_value(const T& value)
    {
        std::stringstream oss;
        oss << value;
        return oss.str();
    }

    template <typename T>
    bool cout(const T& valExpr)
    {
        if (!CTastMgr::GetInstance()->CoutSilent())
        {
            Print("|| %s =~? %s", m_pExpression, cout_value(valExpr).c_str());
        }
        return true;
    }

    template <typename U, typename V>
    bool cout(const U& valExpr, const V& valExpect, bool bPass)
    {
        if (!bPass || !m_bCoutFailOnly && !CTastMgr::GetInstance()->CoutFail())
        {
            Print("|| %s =~? %s [%s]", m_pExpression, cout_value(valExpr).c_str(), pass_cout(bPass));
        }
        if (!bPass)
        {
            Print(">> Expect: %s", cout_value(valExpect).c_str());
            CTastMgr::GetInstance()->AddFail();
            Print(">> Location: [%s:%d](%s)", m_stLocation.file, m_stLocation.line, m_stLocation.function);
        }
        return bPass;
    }

    template <typename U, typename V>
    bool cout(const U& valExpr, const V& valExpect)
    {
        return cout(valExpr, valExpect, valExpr == valExpect);
    }

    bool cout(double valExpr, double valExpect, double limit = 0.0001)
    {
        bool bPass = (valExpr > valExpect) ? ((valExpr - valExpect) <= limit) : ((valExpect - valExpr) <= limit);
        return cout(valExpr, valExpect, bPass);
    }
};

inline
void run_tast(const char* name, PFTAST fun)
{
    CTastMgr::GetInstance()->RunTast(name, fun);
}

} // end of namespace tast

// satement macros used in tast case function
#define SRC_LOCATION tast::SLocation(__FILE__, __LINE__, __FUNCTION__)
#define COUT(expr, ...) tast::CStatement(SRC_LOCATION, #expr).cout((expr), ## __VA_ARGS__)
#define COUTF(expr, ...) tast::CStatement(SRC_LOCATION, #expr, true).cout((expr), ## __VA_ARGS__)
#define COUT_ASSERT(expr, ...) if (!tast::CStatement(SRC_LOCATION, #expr).cout((expr), ## __VA_ARGS__)) return

#define CODE(statement) if (!tast::CTastMgr::GetInstance()->CoutSilent()) {tast::Print("// %s", #statement);} statement
#define DESC(msg, ...) if (!tast::CTastMgr::GetInstance()->CoutSilent()) tast::Print("-- " msg, ## __VA_ARGS__)

// macro to invoke simple tast function void()
#define TAST(fun) tast::run_tast(#fun, fun)
#define TAST_RESULT tast::CTastMgr::GetInstance()->Summary()

// macro to define and run tast object
#define RUN_TAST(...) tast::CTastMgr::GetInstance()->RunTast(__VA_ARGS__)
#define DEF_TAST(name, ...) \
    class CTast_ ## name : public tast::CTastCase \
    { \
    public: \
        CTast_ ## name(const std::string& strDesc) : tast::CTastCase(strDesc) {} \
        virtual void run(); \
    private: \
        static tast::CTastBuilder<CTast_ ## name> m_builder; \
    }; \
    tast::CTastBuilder<CTast_ ## name> CTast_ ## name::m_builder(#name, ## __VA_ARGS__); \
    void CTast_ ## name::run()

#endif /* end of include guard: TINYTAST_HPP__ */
