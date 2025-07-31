#include "tachyon/codegen/machine_generator.hpp"

#include "tachyon/codegen/ir_generator.hpp"
#include "tachyon/common/log.hpp"

#include <sys/mman.h>

#ifndef __x86_64__
#error "machine code compilation for Tachyon only supports x86_64 machines."
#endif

namespace tachyon::codegen {
std::expected<void, Error> generate_machine(IRArena ir, const std::shared_ptr<runtime::Proto> &fn,
                                            runtime::Value *registers) {
    TY_TRACE("generating machine code");
    TY_TRACE(sizeof(runtime::Value) << " " << fn->constants.size());
    TY_TRACE(sizeof(double) << " " << sizeof(bool) << " " << sizeof(std::string) << " "
                            << sizeof(std::shared_ptr<runtime::Proto>) << " " << sizeof(Matrix)
                            << " " << sizeof(unsigned char));

    auto size = 10000; // TODO: fix

    const auto code = static_cast<uint8_t *>(
        mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (code == MAP_FAILED) {
        perror("mmap");
        return std::unexpected(
            Error::create(ErrorKind::MachineGenerationError, SourceSpan(0, 0), "mmap failed"));
    }

    // const auto get = [fn](IRPointer ptr) -> uint16_t {
    //     switch (ptr.kind) {
    //     case IRPointerKind::Constant:
    //         return static_cast<uint16_t>(std::get<double>(fn->constants[ptr.value]));
    //     case IRPointerKind::Register: return ptr.value;
    //     }
    // };

    for (size_t i = 0, j = 0; i < ir.size(); i++) {
        switch (ir[i].kind) {
        case IRNodeKind::Add: {
            // TODO: support xmm8-xmm15
            // TODO: gaurd against something larger than xmm7/15
            // TODO: handle overflow. can we go to stack?

            // TODO: bench whether we should have multiple emplace_backs, etc...
            // mov rax, addr
            code[j++] = 0x48;
            code[j++] = 0xB8;
            switch (ir[i].lhs.kind) {
            case IRPointerKind::Constant:
                *reinterpret_cast<uint64_t *>(code + j) =
                    reinterpret_cast<uint64_t>(fn->constants.data() + ir[i].lhs.value);
                break;
            case IRPointerKind::Register:
                *reinterpret_cast<uint64_t *>(code + j) =
                    reinterpret_cast<uint64_t>(registers + ir[i].lhs.value);
                break;
            }
            j += 8;
            // movsd xmm0, [rax]
            code[j++] = 0xF2;
            code[j++] = 0x0F;
            code[j++] = 0x10;
            code[j++] = 0x00;

            // mov rbx, addr
            code[j++] = 0x48;
            code[j++] = 0xBB;
            switch (ir[i].rhs.kind) {
            case IRPointerKind::Constant:
                *reinterpret_cast<uint64_t *>(code + j) =
                    reinterpret_cast<uint64_t>(fn->constants.data() + ir[i].rhs.value);
                break;
            case IRPointerKind::Register:
                *reinterpret_cast<uint64_t *>(code + j) =
                    reinterpret_cast<uint64_t>(registers + ir[i].rhs.value);
                break;
            }
            j += 8;
            // movsd xmm1, [rbx]
            code[j++] = 0xF2;
            code[j++] = 0x0F;
            code[j++] = 0x10;
            code[j++] = 0x0B;

            // addsd xmm0, xmm1
            code[j++] = 0xF2;
            code[j++] = 0x0F;
            code[j++] = 0x58;
            code[j++] = 0xC1;

            break;
        }
        case IRNodeKind::Ret:
            // TODO: this is not actually RETR/RETC-ing.
            code[j++] = 0xC3;
            break;
        default:
            return std::unexpected(Error::create(
                ErrorKind::InternalError, SourceSpan(0, 0),
                "could not generate machine code for intermediate representation node"));
        }
    }

    if (mprotect(code, size, PROT_READ | PROT_EXEC) != 0) {
        perror("mprotect");
        munmap(code, size);
        return std::unexpected(
            Error::create(ErrorKind::MachineGenerationError, SourceSpan(0, 0), "mprotect failed"));
    }

    fn->compiled = code;
    fn->compiled_length = size;

    return {};
}
} // namespace tachyon::codegen