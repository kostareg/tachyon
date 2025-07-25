#pragma once

#include "tachyon/parser/ast.hpp"
#include "tachyon/runtime/bytecode.hpp"
#include "tachyon/runtime/vm.hpp"

#include <unordered_map>
#include <vector>

namespace tachyon::codegen {
/**
 * @brief generate bytecode from abstract syntax tree
 *
 * Turns the abstract syntax tree into a flat, linear list of bytecode instructions.
 *
 * If `this.errors` is not empty after running the generator, then it cannot be safely assumed that
 * `this.bc` has been properly generated.
 *
 * @see tachyon::parser::Expr
 * @see tachyon::runtime::Bytecode
 */
struct BytecodeGenerator {
  private:
    // TODO: not optimal. consider garbage collection or something to keep track
    //  of the variable so you can free registers when they're not needed
    //  anymore? may need a more complex way to store free registers in that
    //  case. can't even search for std::monostate because people may actually
    //  use the unit type as a value.
    size_t next_free_register = 1;
    uint16_t curr = 0;

  public:
    /// list of bytecode instructions
    std::vector<uint16_t> bc = {};

    /// constant lookup table
    std::vector<runtime::Value> constants = {};

    /// variable name lookup table
    // TODO: is this the best way to do this? consider how it is finding vars by
    //  name. also, since its unique consider std::set.
    std::unordered_map<std::string, size_t> vars = {};

    /// list of generated errors
    std::vector<Error> errors = {};

    /// whether the function is pure (memoizable)
    bool is_pure = true;

    BytecodeGenerator() = default;

    /**
     * @brief preload a variable name lookup table, for function argument names
     * @param vars existing variable name lookup table
     */
    explicit BytecodeGenerator(std::unordered_map<std::string, size_t> vars)
        : vars(std::move(vars)) {
        next_free_register = this->vars.size() + 1;
    };

    void operator()(const parser::LiteralExpr &lit);
    void operator()(const parser::FnExpr &fn);
    void operator()(const parser::UnaryOperatorExpr &unop);
    void operator()(const parser::BinaryOperatorExpr &binop);
    void operator()(const parser::LetExpr &vdecl);
    void operator()(const parser::LetRefExpr &vref);
    void operator()(const parser::MatrixAssignmentExpr &mass);
    void operator()(const parser::MatrixRefExpr &mref);
    void operator()(const parser::FnCallExpr &fnc);
    void operator()(const parser::WhileLoopExpr &wlop);
    void operator()(const parser::BreakExpr &brk);
    void operator()(const parser::ContinueExpr &cont);
    void operator()(const parser::ImportExpr &);
    void operator()(const parser::ReturnExpr &ret);
    void operator()(const parser::MatrixConstructExpr &mc);
    void operator()(const parser::SequenceExpr &seq);
};

/**
 * @brief generate bytecode from abstract syntax tree
 * @param e abstract syntax tree
 * @return function prototype object or error
 */
inline std::expected<runtime::Proto, Error> generate_proto(parser::Expr e) {
    BytecodeGenerator generator;
    std::visit(generator, e.kind);
    if (!generator.errors.empty()) return std::unexpected(Error::createMultiple(generator.errors));
    return runtime::Proto(generator.bc, std::move(generator.constants), 0, generator.is_pure,
                          "<main>", e.span);
}

/**
 * @brief generate bytecode from abstract syntax tree, with preloaded variable names for arguments
 * @param e abstract syntax tree
 * @param args ordered list of argument names
 * @return function prototype object or error
 */
inline std::expected<runtime::Proto, Error> generate_proto(parser::Expr e,
                                                           std::vector<std::string> args) {
    size_t size = args.size();
    std::unordered_map<std::string, size_t> map;
    for (size_t i = 1; i <= size; ++i) {
        map[args[i - 1]] = i;
    }

    BytecodeGenerator generator(std::move(map));
    std::visit(generator, e.kind);
    if (!generator.errors.empty()) return std::unexpected(Error::createMultiple(generator.errors));
    return runtime::Proto(generator.bc, std::move(generator.constants), size, generator.is_pure,
                          "<main>", e.span);
}

/**
 * @brief generate main function bytecode from abstract syntax tree
 * @param e abstract syntax tree
 * @return function prototype object or error
 */
inline std::expected<runtime::Proto, Error> generate_main_proto(parser::Expr e) {
    return generate_proto(std::move(e)).transform([](runtime::Proto proto) {
        proto.is_pure = false;
        proto.bytecode.push_back(runtime::RETV);
        return proto;
    });
}

/**
 * @brief generate repl source bytecode from abstract syntax tree
 * @param e abstract syntax tree
 * @return function prototype object or error
 */
inline std::expected<runtime::Proto, Error> generate_repl_proto(parser::Expr e) {
    return generate_proto(std::move(e)).transform([](runtime::Proto proto) {
        proto.is_pure = false;
        return proto;
    });
}
} // namespace tachyon::codegen