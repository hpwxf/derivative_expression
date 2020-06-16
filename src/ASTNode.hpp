//
// Created by Pascal Havé on 11/06/2020.
//

#ifndef LIBKRIGING_PARSER__ASTNODE_HPP
#define LIBKRIGING_PARSER__ASTNODE_HPP

#include <cassert>
#include <string>
#include <vector>

using Number = double;
using Vector = std::vector<Number>;
using Index =std::size_t;

class NotImplementedException : public std::logic_error {
 public:
  explicit NotImplementedException(const char* func_name)
      : std::logic_error{std::string{func_name} + " is not yet implemented"} {}
};

enum class PriorityLevel : int {
  Unknown,
  Value,
  Factor,
  Term
};

struct IFunction {
  virtual ~IFunction() = default;

 public:
  [[nodiscard]] virtual std::string string() const = 0;
  [[nodiscard]] virtual PriorityLevel level() const = 0;
  
  std::string strHelper(const IFunction & subExpr) const;
};

struct IScalarFunction : IFunction {
  virtual std::unique_ptr<IScalarFunction> clone() const = 0;  
  [[nodiscard]] virtual auto apply(const Vector& x) const -> Number = 0;
  virtual auto diff(const Index I) const -> std::unique_ptr<IScalarFunction> = 0; 
};

struct IVectorFunction : IFunction {
  virtual std::unique_ptr<IVectorFunction> clone() const = 0;
  [[nodiscard]] virtual auto apply(const Vector& x) const -> Vector = 0;
  virtual auto diff(const Index I) const -> std::unique_ptr<IVectorFunction> = 0;
};

#include <tao/pegtl/contrib/parse_tree.hpp>

class ASTNode : public tao::TAO_PEGTL_NAMESPACE::parse_tree::basic_node<ASTNode> {
 public:
  enum class Kind { Unknown, Scalar, Vectorial };

  ASTNode() = default;
  ASTNode(const ASTNode&) = delete;
  ASTNode(ASTNode&&) = delete;
  void operator=(const ASTNode&) = delete;
  void operator=(ASTNode&&) = delete;

 public:
  Kind kind() const { return m_kind; }
  Kind updateKind(Kind kind);

 private:
  Kind m_kind = Kind::Unknown;
};

std::unique_ptr<IScalarFunction> build_function(ASTNode& node);

#endif  // LIBKRIGING_PARSER__ASTNODE_HPP
