#include <memory>
#include <vector>

namespace vm {
/*
 * @brief Function prototype.
 */
struct Proto {
    /// Function name.
    std::string name;

    /// Compiled virtual bytecode.
    uint16_t *bc;

    /// Whether the function returns anything.
    bool returns =
        true; // TODO: remove this and make it declared in the AST or IR.

    /// List of children.
    std::vector<std::unique_ptr<Proto>> children;

    Proto(std::string name, uint16_t *bc) : name(name), bc(bc) {};
    Proto(std::string name, uint16_t *bc,
          std::vector<std::unique_ptr<Proto>> children)
        : name(name), bc(bc), children(std::move(children)) {};
    Proto(std::string name, uint16_t *bc,
          std::vector<std::unique_ptr<Proto>> children, bool returns)
        : name(name), bc(bc), returns(returns),
          children(std::move(children)) {};
};
} // namespace vm
