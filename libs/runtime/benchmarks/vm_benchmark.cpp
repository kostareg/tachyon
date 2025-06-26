#include "tachyon/runtime/bytecode.h"
#include "tachyon/runtime/vm.h"

#include <benchmark/benchmark.h>
#include <fstream>
#include <string>

using namespace tachyon;

// TODO: ./cache_benchmark.cpp

// define a map from strings to bytecode values
static const std::map<std::string, runtime::Bytecode> bytecode_map = {
    /* machine */
    {"RETV", runtime::RETV},
    {"RETC", runtime::RETC},
    {"RETR", runtime::RETR},
    {"NOOP", runtime::NOOP},

    /* register */
    {"LOCR", runtime::LOCR},
    {"LORR", runtime::LORR},

    /* boolean logic */
    {"BNOC", runtime::BNOC},
    {"BNOR", runtime::BNOR},
    {"BACC", runtime::BACC},
    {"BOCC", runtime::BOCC},
    {"BARC", runtime::BARC},
    {"BORC", runtime::BORC},
    {"BACR", runtime::BACR},
    {"BOCR", runtime::BOCR},
    {"BARR", runtime::BARR},
    {"BORR", runtime::BORR},

    /* positional */
    {"JMPU", runtime::JMPU},
    {"JMCI", runtime::JMCI},
    {"JMCN", runtime::JMCN},
    {"JMRI", runtime::JMRI},
    {"JMRN", runtime::JMRN},

    /* arithmetic */
    {"MACC", runtime::MACC},
    {"MSCC", runtime::MSCC},
    {"MMCC", runtime::MMCC},
    {"MDCC", runtime::MDCC},
    {"MPCC", runtime::MPCC},
    {"MARC", runtime::MARC},
    {"MSRC", runtime::MSRC},
    {"MMRC", runtime::MMRC},
    {"MDRC", runtime::MDRC},
    {"MPRC", runtime::MPRC},
    {"MACR", runtime::MACR},
    {"MSCR", runtime::MSCR},
    {"MMCR", runtime::MMCR},
    {"MDCR", runtime::MDCR},
    {"MPCR", runtime::MPCR},
    {"MARR", runtime::MARR},
    {"MSRR", runtime::MSRR},
    {"MMRR", runtime::MMRR},
    {"MDRR", runtime::MDRR},
    {"MPRR", runtime::MPRR},

    /* comparison */
    {"CECC", runtime::CECC},
    {"CNCC", runtime::CNCC},
    {"CLCC", runtime::CLCC},
    {"CGCC", runtime::CGCC},
    {"CHCC", runtime::CHCC},
    {"CFCC", runtime::CFCC},
    {"CERC", runtime::CERC},
    {"CNRC", runtime::CNRC},
    {"CLRC", runtime::CLRC},
    {"CGRC", runtime::CGRC},
    {"CHRC", runtime::CHRC},
    {"CFRC", runtime::CFRC},
    {"CECR", runtime::CECR},
    {"CNCR", runtime::CNCR},
    {"CLCR", runtime::CLCR},
    {"CGCR", runtime::CGCR},
    {"CHCR", runtime::CHCR},
    {"CFCR", runtime::CFCR},
    {"CERR", runtime::CERR},
    {"CNRR", runtime::CNRR},
    {"CLRR", runtime::CLRR},
    {"CGRR", runtime::CGRR},
    {"CHRR", runtime::CHRR},
    {"CFRR", runtime::CFRR},

    /* function */
    {"CALC", runtime::CALC},
    {"CALR", runtime::CALR},

    /* intrinsic */
    {"PRNC", runtime::PRNC},
    {"PRNR", runtime::PRNR},
};

class VMDataFixture : public benchmark::Fixture
{
  public:
    void SetUp(const ::benchmark::State &) override
    {
        const std::string included_files[] = {
            "00-blank",
            "01-basic",
        };

        for (const std::string &included_file : included_files)
        {
            const std::string included_file_path =
                "benchmark-data/" + included_file + ".tachyon-bytes";
            std::ifstream file(included_file_path, std::ios::binary | std::ios::ate);
            std::streamsize size = file.tellg();
            if (size == -1)
                throw std::runtime_error("could not find file: " + included_file);
            file.seekg(0, std::ios::beg);
            std::string content(size, '\0');
            file.read(content.data(), size);

            std::vector<uint8_t> instructions;
            // parse content as bytecode or numbers
            for (size_t i = 0; i < content.size(); ++i)
            {
                size_t start = i;

                // continue reading until space or newline
                while (i < content.size() && content[i] != ' ' && content[i] != '\n')
                    ++i;

                std::string instruction = content.substr(start, i - start);

                if (std::isdigit(instruction[0]))
                {
                    // parse as number
                    uint8_t number = std::stoi(instruction);
                    instructions.push_back(number);
                }
                else
                {
                    // parse as Bytecode
                    runtime::Bytecode number = bytecode_map.at(instruction);
                    instructions.push_back(number);
                }
            }

            vm_data[included_file] = std::move(instructions);
        }
    }

    void TearDown(const ::benchmark::State &) override { vm_data.clear(); }

  protected:
    std::unordered_map<std::string, std::vector<uint8_t>> vm_data;
};

BENCHMARK_F(VMDataFixture, RunBlank)(benchmark::State &state)
{
    std::vector<uint8_t> _00_blank = vm_data["00-blank"];
    const runtime::Proto proto(_00_blank, {}, 0, false, "<main>", SourceSpan(0, 0));

    for (auto _ : state)
    {
        runtime::VM vm;
        vm.run(proto).value();
        benchmark::DoNotOptimize(vm);
    }
};

BENCHMARK_F(VMDataFixture, RunBasic)(benchmark::State &state)
{
    std::vector<uint8_t> _01_basic = vm_data["01-basic"];
    const runtime::Proto proto(_01_basic, {runtime::Value(1.), runtime::Value(2.)}, 0, false,
                               "<main>", SourceSpan(0, 0));

    for (auto _ : state)
    {
        runtime::VM vm;
        vm.run(proto).value();
        benchmark::DoNotOptimize(vm);
    }
};

BENCHMARK_MAIN();
