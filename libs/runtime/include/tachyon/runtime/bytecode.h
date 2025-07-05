#pragma once

#include <cstdint>

namespace tachyon::runtime
{
/**
 * @brief bytecode values
 *
 * +-------+-----------------+
 * | start | operation class |
 * +-------+-----------------+
 * | 0x00  | machine         |
 * | 0x10  | register        |
 * | 0x20  | stack           |
 * | 0x30  | boolean logic   |
 * | 0x40  | positional      |
 * | 0x50  | arithmetic      |
 * | 0x70  | comparison      |
 * | 0x90  | lists           |
 * | 0xE0  | function        |
 * | 0xF0  | intrinsic       |
 * +-------+-----------------+
 */
enum Bytecode : uint16_t
{
    /* machine */
    RETV = 0x00, // return void
    RETC = 0x01, // return constant
    RETR = 0x02, // return register
    NOOP = 0x0F, // no-op

    /* register */
    LOCR = 0x10, // load constant -> register
    LORR = 0x11, // load register -> register
    // LORS = 0x12, // load register <- stack

    /* stack */
    // PUSC = 0x20, // push stack <- constant
    // PUSR = 0x21, // push stack <- register
    // POPS = 0x2E, // pop stack

    /* boolean logic */
    BNOC = 0x30, //          NOT constant -> register
    BNOR = 0x31, //          NOT register -> register
    BACC = 0x32, // constant AND constant -> register
    BOCC = 0x33, // constant  OR constant -> register
    BARC = 0x34, // register AND constant -> register
    BORC = 0x35, // register  OR constant -> register
    BACR = 0x36, // constant AND register -> register
    BOCR = 0x37, // constant  OR register -> register
    BARR = 0x38, // register AND register -> register
    BORR = 0x39, // register  OR register -> register

    /* positional */
    JMPU = 0x40, // jump unconditionally to bytecode address
    JMCI = 0x41, // jump if     constant to bytecode address
    JMCN = 0x42, // jump if   ! constant to bytecode address
    JMRI = 0x43, // jump if     register to bytecode address
    JMRN = 0x44, // jump if   ! register to bytecode address

    /* arithmetic */
    MACC = 0x50, // constant +  constant -> register
    MSCC = 0x51, // constant -  constant -> register
    MMCC = 0x52, // constant *  constant -> register
    MDCC = 0x53, // constant /  constant -> register
    MPCC = 0x54, // constant ^  constant -> register
    MARC = 0x55, // register +  constant -> register
    MSRC = 0x56, // register -  constant -> register
    MMRC = 0x57, // register *  constant -> register
    MDRC = 0x58, // register /  constant -> register
    MPRC = 0x59, // register ^  constant -> register
    MACR = 0x5A, // constant +  register -> register
    MSCR = 0x5B, // constant -  register -> register
    MMCR = 0x5C, // constant *  register -> register
    MDCR = 0x5D, // constant /  register -> register
    MPCR = 0x5E, // constant ^  register -> register
    MARR = 0x5F, // register +  register -> register
    MSRR = 0x60, // register -  register -> register
    MMRR = 0x61, // register *  register -> register
    MDRR = 0x62, // register /  register -> register
    MPRR = 0x63, // register ^  register -> register

    // TODO: negate, inverse, pow, nth root/sqrt, abs, floor, ceil, round,
    //  min?/max?, trig functions, potentially shortcuts for square and cube.
    //  but i may also just implement these manually? like the trig functions at
    //  least are for sure doable. consider checking performance b/w cmath's
    //  trig functions and a tachyon trig function.

    /* comparison */
    CECC = 0x70, // constant == constant -> register
    CNCC = 0x71, // constant != constant -> register
    CLCC = 0x72, // constant <  constant -> register
    CGCC = 0x73, // constant >  constant -> register
    CHCC = 0x74, // constant <= constant -> register
    CFCC = 0x75, // constant >= constant -> register
    CERC = 0x76, // register == constant -> register
    CNRC = 0x77, // register != constant -> register
    CLRC = 0x78, // register <  constant -> register
    CGRC = 0x79, // register >  constant -> register
    CHRC = 0x7A, // register <= constant -> register
    CFRC = 0x7B, // register >= constant -> register
    CECR = 0x7C, // constant == register -> register
    CNCR = 0x7D, // constant != register -> register
    CLCR = 0x7E, // constant <  register -> register
    CGCR = 0x7F, // constant >  register -> register
    CHCR = 0x80, // constant <= register -> register
    CFCR = 0x81, // constant >= register -> register
    CERR = 0x82, // register == register -> register
    CNRR = 0x83, // register != register -> register
    CLRR = 0x84, // register <  register -> register
    CGRR = 0x85, // register >  register -> register
    CHRR = 0x86, // register <= register -> register
    CFRR = 0x87, // register >= register -> register

    /* lists */
    LIUC = 0x90, // push constant                              -> register
    LIUR = 0x91, // push register                              -> register
    LIOR = 0x92, // pop  register                              -> register
    GRRC = 0x93, // get register row register col constant val of register
    GRRR = 0x94, // get register row register col register val of register
    SCCC = 0x95, // set constant row constant col constant val of register
    SRCC = 0x96, // set register row constant col constant val of register
    SRRC = 0x97, // set register row register col constant val of register
    SRRR = 0x98, // set register row register col register val of register
    SCRR = 0x99, // set constant row register col register val of register
    SCCR = 0x9A, // set constant row constant col register val of register
    SCRC = 0x9B, // set constant row register col constant val of register
    SRCR = 0x9C, // set register row constant col register val of register

    /* function */
    CALC = 0xE0, // call constant fn
    CALR = 0xE1, // call register fn

    /* intrinsic */
    PRNC = 0xF0, // print constant
    PRNR = 0xF1, // print register
    // TODO: type(x) -> "num", sizeof(x), exit(), time()?, str()?
};
} // namespace tachyon::runtime