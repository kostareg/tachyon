#include <memory>
#include <vector>

/*
 * @brief Function prototype.
 */
struct Proto {
    /// Function name.
    std::string name;

    /// Compiled virtual bytecode.
    uint16_t *bc;

    // TODO: needed?
    /// Number of paramaters. The first n registers will be allocated to params,
    /// in order.
    unsigned int params;

    unsigned int returns;

    // TODO: vector?
    /// List of children.
    std::vector<std::unique_ptr<Proto>> children;

    Proto(std::string name, uint16_t *bc) : name(name), bc(bc) {
        params = 0;
        returns = 0;
    };
    Proto(std::string name, uint16_t *bc, unsigned int params, unsigned int returns)
        : name(name), bc(bc), params(params), returns(returns) {};
    Proto(std::string name, uint16_t *bc, std::vector<std::unique_ptr<Proto>> children)
        : name(name), bc(bc), children(std::move(children)) {
        params = 0;
    };
    Proto(std::string name, uint16_t *bc, unsigned int params, unsigned int returns,
          std::vector<std::unique_ptr<Proto>> children)
        : name(name), bc(bc), params(params), returns(returns), children(std::move(children)) {};
};
