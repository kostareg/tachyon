#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "vm/vm.hpp"

int main(int argc, char *argv[]) {
#ifdef DEBUG
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace); // Enable trace messages
    console_sink->set_pattern("%^%Y-%m-%d %H:%M:%S.%e [%L] %v%$");

    // Create a logger with the colored sink
    auto logger = std::make_shared<spdlog::logger>("colored_logger", console_sink);
    spdlog::set_default_logger(logger);

    // Set global log level to trace
    spdlog::set_level(spdlog::level::trace);
#endif
    uint16_t fn1[0xFFFF] = {};
    fn1[0] = 0x0001;
    fn1[1] = 0x0010;
    fn1[2] = 0;
    fn1[3] = 10;
    fn1[4] = 0x0011;
    fn1[5] = 1;
    fn1[6] = 0;
    fn1[7] = 0x005C;
    fn1[8] = 0;
    fn1[9] = 1;
    fn1[10] = 0;
    fn1[11] = 0x0000;

    uint16_t fn2[0xFFFF] = {};
    fn2[0] = 0x0001;
    fn2[1] = 0x005A;
    fn2[2] = 0;
    fn2[3] = 1;
    fn2[4] = 0;
    fn2[5] = 0x0004;
    fn2[6] = 0;

    uint16_t main[0xFFFF] = {};
    main[0] = 0x0001;
    main[1] = 0x0100;
    main[2] = 0;
    main[3] = 0;
    main[4] = 0x0110;
    main[5] = 0;
    main[6] = 0x0001;
    main[7] = 0x0120;
    main[8] = 0;
    main[9] = 10;
    main[10] = 0x0120;
    main[11] = 1;
    main[12] = 20;
    main[13] = 0x0100;
    main[14] = 2;
    main[15] = 1;
    main[16] = 0x0110;
    main[17] = 2;
    main[18] = 0x0010;
    main[19] = 1;
    main[20] = 0xFFFF;
    main[21] = 0x0110;
    main[22] = 1;
    main[23] = 0x0000;

    std::vector<std::unique_ptr<vm::Proto>> a;
    a.push_back(std::make_unique<vm::Proto>("fn1", fn1));
    a.push_back(std::make_unique<vm::Proto>("fn2", fn2, 2, 1));
    vm::Proto main_proto("main", main, std::move(a));

    vm::VM vm;
    vm.run_fn(&main_proto);
    // std::cout << vm.registers[0] << std::endl;
    // std::cout << vm.registers[1] << std::endl;

    return 0;
}
