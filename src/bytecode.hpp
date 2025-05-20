enum Bytecode {
    EXIT = 0x0000,
    NOOP = 0x0001,
    RETV = 0x0002,
    RETC = 0x0003,
    RETR = 0x0004,

    LORC = 0x0010,
    LORR = 0x0011,
    LORS = 0x0012,

    PUSC = 0x0020,
    PUSR = 0x0021,
    POPS = 0x002E,

    CRLC = 0x0030,
    CRGC = 0x0031,
    CRLR = 0x0032,
    CRGR = 0x0033,
    CREC = 0x0034,
    CRER = 0x0035,

    JMPC = 0x0040,
    JMPR = 0x0041,
    JMPS = 0x0042,

    // TODO: if statements

    MARC = 0x0050,
    MSRC = 0x0051,
    MMRC = 0x0052,
    MDRC = 0x0053,
    MPRC = 0x0054,
    MACR = 0x0055,
    MSCR = 0x0056,
    MMCR = 0x0057,
    MDCR = 0x0058,
    MPCR = 0x0059,
    MARR = 0x005A,
    MSRR = 0x005B,
    MMRR = 0x005C,
    MDRR = 0x005D,
    MPRR = 0x005E,

    // TODO: negate and inverse, potentially shortcuts for square and cube

    CALR = 0x0100,
    PUHC = 0x0110,
    PUHR = 0x0111,
};
