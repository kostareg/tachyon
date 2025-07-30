#include "tachyon/codegen/machine_generator.hpp"

#include "tachyon/common/log.hpp"

#include <sys/mman.h>

#ifndef __x86_64__
#error "just in time compilation for Tachyon only supports x86_64 machines."
#endif

namespace tachyon::codegen {
std::expected<void, tachyon::Error> generate_machine(std::shared_ptr<runtime::Proto> fn) {
    TY_TRACE("generating machine code");

    unsigned char code[] = {
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x40, // mov rax, 0x4058C00000000000
        0x66, 0x48, 0x0F, 0x6E, 0xC0,                               // movq xmm0, rax
        0xC3                                                        // ret
    };

    // allocate executable memory
    void *mem =
        mmap(nullptr, sizeof(code), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        perror("mmap");
        return std::unexpected(
            Error::create(ErrorKind::MachineGenerationError, SourceSpan(0, 0), "mmap failed"));
    }
    memcpy(mem, code, sizeof(code));
    if (mprotect(mem, sizeof(code), PROT_READ | PROT_EXEC) != 0) {
        perror("mprotect");
        munmap(mem, sizeof(code));
        return std::unexpected(
            Error::create(ErrorKind::MachineGenerationError, SourceSpan(0, 0), "mprotect failed"));
    }

    fn->compiled = mem;
    fn->compiled_length = sizeof(code);

    return {};
}
} // namespace tachyon::codegen