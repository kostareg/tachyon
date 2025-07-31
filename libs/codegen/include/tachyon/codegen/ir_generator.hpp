#pragma once
#include "tachyon/common/error.hpp"
#include "tachyon/runtime/proto.hpp"

#include <cstdint>
#include <expected>
#include <utility>
#include <vector>

namespace tachyon::codegen {
enum class IRPointerKind {
    Constant,
    Register,
};

struct IRPointer {
    IRPointerKind kind;
    uint16_t value;
};

inline IRPointer make_register(const uint16_t value) {
    return IRPointer{IRPointerKind::Register, value};
}

inline IRPointer make_constant(const uint16_t value) {
    return IRPointer{IRPointerKind::Constant, value};
}

enum class IRNodeKind {
    Add,
    Sub,
    Mul,
    Div,
    Ret,
};

struct IRNode {
    IRNodeKind kind;
    IRPointer lhs;
    IRPointer rhs;
    IRPointer dst;
};

class IRArena {
    std::vector<IRNode> nodes;

  public:
    void alloc(const IRNode &node) { nodes.push_back(node); }

    template <class... Args> void emplace(Args &&...args) {
        nodes.emplace_back(std::forward<Args>(args)...);
    }

    [[nodiscard]] size_t size() const { return nodes.size(); }

    IRNode &operator[](const size_t i) { return nodes[i]; }
};

std::expected<IRArena, Error> generate_ir(const std::shared_ptr<runtime::Proto> &);
} // namespace tachyon::codegen