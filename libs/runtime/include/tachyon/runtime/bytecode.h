#pragma once

#include <cstdint>

namespace tachyon::runtime
{
/**
 * @brief bytecode values
 *
 * +-------+-----------------+
 * | Start | Operation class |
 * +-------+-----------------+
 * | 0x00  | Machine         |
 * | 0x10  | Register        |
 * | 0x20  | Stack           |
 * | 0x30  | Comparison      |
 * | 0x40  | Positional      |
 * | 0x50  | Arithmetic      |
 * | 0xE0  | Function        |
 * | 0xF0  | Intrinsics      |
 * +-------+-----------------+
 */
enum Bytecode : uint8_t
{
    RETV = 0x00, // return void
    RETC = 0x01, // return constant
    RETR = 0x02, // return register
    NOOP = 0x0F, // no-op

    LOCR = 0x10, // load constant -> register
    LORR = 0x11, // load register -> register
    // LORS = 0x12, // load register <- stack

    // PUSC = 0x20, // push stack <- constant
    // PUSR = 0x21, // push stack <- register
    // POPS = 0x2E, // pop stack

    CRLC = 0x30, // (register  < constant) -> register
    CRGC = 0x31, // (register  > constant) -> register
    CRLR = 0x32, // (register  < register) -> register
    CRGR = 0x33, // (register  > register) -> register
    CREC = 0x34, // (register == constant) -> register
    CRER = 0x35, // (register == register) -> register

    JMPC = 0x40, // jump to constant address
    JMPR = 0x41, // jump to register address
    // JMPS = 0x42, // jump to stack address

    // TODO: if statements

    MACC = 0x50, // constant + constant -> register
    MSCC = 0x51, // constant - constant -> register
    MMCC = 0x52, // constant * constant -> register
    MDCC = 0x53, // constant / constant -> register
    MARC = 0x54, // register + constant -> register
    MSRC = 0x55, // register - constant -> register
    MMRC = 0x56, // register * constant -> register
    MDRC = 0x57, // register / constant -> register
    MACR = 0x58, // constant + register -> register
    MSCR = 0x59, // constant - register -> register
    MMCR = 0x5A, // constant * register -> register
    MDCR = 0x5B, // constant / register -> register
    MARR = 0x5C, // register + register -> register
    MSRR = 0x5D, // register - register -> register
    MMRR = 0x5E, // register * register -> register
    MDRR = 0x5F, // register / register -> register

    // TODO: negate, inverse, pow, nth root/sqrt, abs, floor, ceil, round,
    //  min?/max?, trig functions, potentially shortcuts for square and cube.
    //  but i may also just implement these manually? like the trig functions at
    //  least are for sure doable. consider checking performance b/w cmath's
    //  trig functions and a tachyon trig function.

    CALC = 0xE0, // call constant fn
    CALR = 0xE1, // call register fn

    // TODO: type(x) -> "num", sizeof(x), exit(), time()?, str()?
    PRNC = 0xF0, // print constant
    PRNR = 0xF1, // print register
};
} // namespace tachyon::runtime