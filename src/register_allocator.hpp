#include <cstdint>
#include <mimalloc.h>

/* TODO: consider implementation.
 * My issue here is that when mi_realloc is called to shrink, the pointer may
 * change. The current implementation of VM just keeps the pointer where it is.
 *
 * Consider having each function store their offset, then using
 *
 * ```
 * int* get_register(size_t index) {
 *     return registers + index;  // Offset by 'index'
 * }
 * ```
 */

class RegisterAllocator {
    /**
     * @brief 16-bit registers.
     */
    uint16_t *registers;

    /**
     * @brief length of registers.
     */
    size_t reglast;

    /**
     * @brief register capacity.
     */
    size_t cap;

  public:
    RegisterAllocator(size_t initial_cap = 1024) : cap(initial_cap) {
        registers = static_cast<uint16_t *>(mi_malloc(cap * sizeof(uint16_t)));
        reglast = 0;
    }

    uint16_t *new_fn() {
        space_fn();

        uint16_t *r = registers + reglast;
        reglast += 256;
        return r;
    }

    /**
     * Pushes one value to the start of the next set of registers.
     */
    void early_push(size_t off, uint16_t val) {
        space_fn();

        registers[reglast + off] = val;
    }

    /*
     * @brief Make sure there's enough space for an upcoming function.
     */
    void space_fn() {
        if (reglast >= cap) {
            cap *= 2;
            registers = static_cast<uint16_t *>(mi_realloc(registers, cap * sizeof(uint16_t)));
        }
    }

    void free_fn() {
        reglast -= 256;

        /*
        if (cap / 2 >= reglast) {
            cap /= 2;
            spdlog::trace("new cap: {}", cap);
            spdlog::trace("first elem: {}", registers[0]);
            spdlog::trace("last elem: {}", registers[cap]);
            registers = static_cast<uint16_t *>(mi_realloc(registers, cap * sizeof(uint16_t)));
            spdlog::trace("first elem: {}", registers[0]);
            spdlog::trace("last elem: {}", registers[cap]);
        }
        */
    }

    ~RegisterAllocator() { mi_free(registers); }
};
