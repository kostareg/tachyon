#include "tachyon/runtime/value.h"
#include "tachyon/runtime/proto.h"

namespace tachyon::runtime {
/**
 * @brief small visitor for printing values
 */
void printValue(const Value &reg) {
    std::visit(
        []<typename T0>(const T0 &val) {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                std::print("()");
            } else if constexpr (std::is_same_v<T, std::shared_ptr<Proto>>) {
                std::print("function prototype '{}'",
                           static_cast<std::shared_ptr<Proto>>(val)->name);
            } else if constexpr (std::is_same_v<T, Matrix>) {
                std::print("[");
                for (size_t i = 1; i <= val.getCurrent(); ++i) {
                    if (i > 1) std::print(", ");
                    std::print("{}", val(i));
                    if (i == val.getWidth()) std::print(";");
                }
                std::print("]");
            } else {
                std::print("{}", val);
            }
        },
        reg);
}
} // namespace tachyon::runtime