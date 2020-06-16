//
// Created by Pascal Havé on 11/06/2020.
//

#include "ASTNode.hpp"
#include <iostream>
#include <utility>
#include "grammar_symbol.hpp"

class ScalarUndefFunction : public IScalarFunction {
 public:
  explicit ScalarUndefFunction(std::string s) : m_string(std::move(s)) {}

  [[nodiscard]] std::string string() const override { return "undef_scalar(" + m_string + ")"; }
  [[nodiscard]] Number apply(const Vector& x) const override { throw NotImplementedException(__PRETTY_FUNCTION__); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Unknown; }

 private:
  std::string m_string;
};

class VectorUndefFunction : public IVectorFunction {
 public:
  explicit VectorUndefFunction(std::string s) : m_string(std::move(s)) {}

  [[nodiscard]] std::string string() const override { return "undef_vector(" + m_string + ")"; }
  [[nodiscard]] Vector apply(const Vector& x) const override { throw NotImplementedException(__PRETTY_FUNCTION__); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Unknown; }

 private:
  std::string m_string;
};

class ScalarAdd : public IScalarFunction {
 public:
  explicit ScalarAdd(std::unique_ptr<IScalarFunction>&& a, std::unique_ptr<IScalarFunction>&& b)
      : m_a(std::move(a)), m_b(std::move(b)) {}

  [[nodiscard]] std::string string() const override { return strHelper(*m_a) + "+" + strHelper(*m_b); }
  [[nodiscard]] Number apply(const Vector& x) const override { return m_a->apply(x) + m_b->apply(x); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Term; }

 private:
  std::unique_ptr<IScalarFunction> m_a;
  std::unique_ptr<IScalarFunction> m_b;
};

class ScalarSub : public IScalarFunction {
 public:
  explicit ScalarSub(std::unique_ptr<IScalarFunction>&& a, std::unique_ptr<IScalarFunction>&& b)
      : m_a(std::move(a)), m_b(std::move(b)) {}

  [[nodiscard]] std::string string() const override { return strHelper(*m_a) + "-" + strHelper(*m_b); }
  [[nodiscard]] Number apply(const Vector& x) const override { return m_a->apply(x) - m_b->apply(x); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Term; }

 private:
  std::unique_ptr<IScalarFunction> m_a;
  std::unique_ptr<IScalarFunction> m_b;
};

class VectorAdd : public IVectorFunction {
 public:
  explicit VectorAdd(std::unique_ptr<IVectorFunction>&& a, std::unique_ptr<IVectorFunction>&& b)
      : m_a(std::move(a)), m_b(std::move(b)) {}

  [[nodiscard]] std::string string() const override { return strHelper(*m_a) + "+" + strHelper(*m_b); }
  [[nodiscard]] Vector apply(const Vector& x) const override { return impl(m_a->apply(x), m_b->apply(x)); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Term; }

 private:
  std::unique_ptr<IVectorFunction> m_a;
  std::unique_ptr<IVectorFunction> m_b;

 public:
  static Vector impl(Vector a, const Vector& b) {
    assert(a.size() == b.size());
    Vector result{};
    for (std::size_t i = 0; i < a.size(); ++i) {
      a[i] += b[i];
    }
    return a;
  }
};

class VectorSub : public IVectorFunction {
 public:
  explicit VectorSub(std::unique_ptr<IVectorFunction>&& a, std::unique_ptr<IVectorFunction>&& b)
      : m_a(std::move(a)), m_b(std::move(b)) {}

  [[nodiscard]] std::string string() const override { return strHelper(*m_a) + "-" + strHelper(*m_b); }
  [[nodiscard]] Vector apply(const Vector& x) const override { return impl(m_a->apply(x), m_b->apply(x)); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Term; }

 private:
  std::unique_ptr<IVectorFunction> m_a;
  std::unique_ptr<IVectorFunction> m_b;

 public:
  static Vector impl(Vector a, const Vector& b) {
    assert(a.size() == b.size());
    Vector result{};
    for (std::size_t i = 0; i < a.size(); ++i) {
      a[i] -= b[i];
    }
    return a;
  }
};

class ScalarPrefixPlus : public IScalarFunction {
 public:
  explicit ScalarPrefixPlus(std::unique_ptr<IScalarFunction>&& a) : m_a(std::move(a)) {}

  [[nodiscard]] std::string string() const override { return "+" + strHelper(*m_a); }
  [[nodiscard]] Number apply(const Vector& x) const override { return m_a->apply(x); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Value; }

 private:
  std::unique_ptr<IScalarFunction> m_a;
};

class ScalarPrefixMinus : public IScalarFunction {
 public:
  explicit ScalarPrefixMinus(std::unique_ptr<IScalarFunction>&& a) : m_a(std::move(a)) {}

  [[nodiscard]] std::string string() const override { return "-" + strHelper(*m_a); }
  [[nodiscard]] Number apply(const Vector& x) const override { return -m_a->apply(x); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Value; }

 private:
  std::unique_ptr<IScalarFunction> m_a;
};

class VectorPrefixPlus : public IVectorFunction {
 public:
  explicit VectorPrefixPlus(std::unique_ptr<IVectorFunction>&& a) : m_a(std::move(a)) {}

  [[nodiscard]] std::string string() const override { return "+" + strHelper(*m_a); }
  [[nodiscard]] Vector apply(const Vector& x) const override { return m_a->apply(x); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Value; }

 private:
  std::unique_ptr<IVectorFunction> m_a;
};

class VectorPrefixMinus : public IVectorFunction {
 public:
  explicit VectorPrefixMinus(std::unique_ptr<IVectorFunction>&& a) : m_a(std::move(a)) {}

  [[nodiscard]] std::string string() const override { return "-" + strHelper(*m_a); }
  [[nodiscard]] Vector apply(const Vector& x) const override { return impl(m_a->apply(x)); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Value; }

 private:
  std::unique_ptr<IVectorFunction> m_a;

 public:
  static Vector impl(Vector a) {
    for (double & ai : a) {
      ai *= -1;
    }
    return a;
  }
};

class ScalarScalarProduct : public IScalarFunction {
 public:
  explicit ScalarScalarProduct(std::unique_ptr<IScalarFunction>&& a, std::unique_ptr<IScalarFunction>&& b)
      : m_a(std::move(a)), m_b(std::move(b)) {}

  [[nodiscard]] std::string string() const override { return strHelper(*m_a) + "*" + strHelper(*m_b); }
  [[nodiscard]] Number apply(const Vector& x) const override { return m_a->apply(x) * m_b->apply(x); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Value; }

 private:
  std::unique_ptr<IScalarFunction> m_a;
  std::unique_ptr<IScalarFunction> m_b;
};

class ScalarVectorProduct : public IVectorFunction {
 public:
  explicit ScalarVectorProduct(std::unique_ptr<IScalarFunction>&& a, std::unique_ptr<IVectorFunction>&& b)
      : m_a(std::move(a)), m_b(std::move(b)) {}

  [[nodiscard]] std::string string() const override { return strHelper(*m_a) + "*" + strHelper(*m_b); }
  [[nodiscard]] Vector apply(const Vector& x) const override { return impl(m_a->apply(x), m_b->apply(x)); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Factor; }

 private:
  std::unique_ptr<IScalarFunction> m_a;
  std::unique_ptr<IVectorFunction> m_b;

 public:
  static Vector impl(const Number a, Vector b) {
    for (std::size_t i = 0; i < b.size(); ++i) {
      b[i] *= a;
    }
    return b;
  }
};

class ScalarScalarDivide : public IScalarFunction {
 public:
  explicit ScalarScalarDivide(std::unique_ptr<IScalarFunction>&& a, std::unique_ptr<IScalarFunction>&& b)
      : m_a(std::move(a)), m_b(std::move(b)) {}

  [[nodiscard]] std::string string() const override { return strHelper(*m_a) + "/" + strHelper(*m_b); }
  [[nodiscard]] Number apply(const Vector& x) const override { return m_a->apply(x) / m_b->apply(x); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Factor; }

 private:
  std::unique_ptr<IScalarFunction> m_a;
  std::unique_ptr<IScalarFunction> m_b;
};

class DotProduct : public IScalarFunction {
 public:
  explicit DotProduct(std::unique_ptr<IVectorFunction>&& a, std::unique_ptr<IVectorFunction>&& b)
      : m_a(std::move(a)), m_b(std::move(b)) {}

  [[nodiscard]] std::string string() const override { return "dot(" + m_a->string() + "," + m_b->string() + ")"; }
  [[nodiscard]] Number apply(const Vector& x) const override { return impl(m_a->apply(x), m_b->apply(x)); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Value; }

 private:
  std::unique_ptr<IVectorFunction> m_a;
  std::unique_ptr<IVectorFunction> m_b;

 public:
  static Number impl(const Vector& a, const Vector& b) {
    assert(a.size() == b.size());
    Number result = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
      result += a[i] * b[i];
    }
    return result;
  }
};

class ExpFunction : public IScalarFunction {
 public:
  explicit ExpFunction(std::unique_ptr<IScalarFunction>&& a) : m_a(std::move(a)) {}

  [[nodiscard]] std::string string() const override { return "exp(" + m_a->string() + ")"; }
  [[nodiscard]] Number apply(const Vector& x) const override { return exp(m_a->apply(x)); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Value; }

 private:
  std::unique_ptr<IScalarFunction> m_a;
};

class ScalarNorm : public IScalarFunction {
 public:
  explicit ScalarNorm(std::unique_ptr<IVectorFunction>&& a) : m_a(std::move(a)) {}

  [[nodiscard]] std::string string() const override { return "norm2(" + m_a->string() + ")"; }
  [[nodiscard]] Number apply(const Vector& x) const override { return DotProduct::impl(x, x); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Value; }

 private:
  std::unique_ptr<IVectorFunction> m_a;
};

class VectorIdentity : public IVectorFunction {
 public:
  explicit VectorIdentity(std::string s) : m_s(std::move(s)) {}

  [[nodiscard]] std::string string() const override { return m_s; }
  [[nodiscard]] Vector apply(const Vector& x) const override { return x; }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Value; }

 private:
  std::string m_s;
};

class IndexedVectorIdentity : public IScalarFunction {
 public:
  explicit IndexedVectorIdentity(std::unique_ptr<IVectorFunction>&& a, const std::string& index)
      : m_a(std::move(a)), m_index(std::stoul(index)) {}

  [[nodiscard]] std::string string() const override { return strHelper(*m_a) + "_" + std::to_string(m_index); }
  [[nodiscard]] Number apply(const Vector& x) const override { return impl(m_a->apply(x), m_index); }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Value; }

 private:
  std::unique_ptr<IVectorFunction> m_a;
  std::size_t m_index;

 public:
  static Number impl(const Vector& a, const std::size_t index) {
    assert(index < a.size());
    return a[index];
  }
};

class ScalarNumber : public IScalarFunction {
 public:
  explicit ScalarNumber(std::string s) : m_s(std::move(s)), m_number(std::stod(m_s)) {}

  [[nodiscard]] std::string string() const override { return m_s; }
  [[nodiscard]] auto apply(const Vector& x) const -> Number override { return m_number; }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Value; }

 private:
  std::string m_s;  //! keep string to get exact registered form
  Number m_number;
};

class ScalarValue : public IScalarFunction {
 public:
  explicit ScalarValue(std::string s) : m_s(std::move(s)) {}

  [[nodiscard]] std::string string() const override { return m_s; }
  [[nodiscard]] Number apply(const Vector& x) const override {
    if (m_s == "pi") {
      return 4 * atan(1);
    } else if (m_s == "e") {
      return exp(1);
    } else {
      throw NotImplementedException(__PRETTY_FUNCTION__);
    }
  }
  [[nodiscard]] PriorityLevel level() const override { return PriorityLevel::Value; }

 private:
  std::string m_s;
};

namespace {
ASTNode::Kind mark_data_kind(ASTNode& node) {
  if (node.is_root()) {
    return node.updateKind(mark_data_kind(*node.children.front()));
  } else if (node.is<language::scalar_variable>() || node.is<language::scalar_constant>()
             || node.is<language::number>()) {
    return node.updateKind(ASTNode::Kind::Scalar);
  } else if (node.is<language::vector_variable>()) {
    return node.updateKind(ASTNode::Kind::Vectorial);
  } else if (node.is<language::indexed_vector_variable>()) {
    mark_data_kind(*node.children[0]);
    return node.updateKind(ASTNode::Kind::Scalar);
  } else if (node.is<language::prefix_minus>() || node.is<language::prefix_plus>()) {
    return node.updateKind(mark_data_kind(*node.children[0]));
  } else if (node.is<language::unary_s2s_function_name>() || node.is<language::unary_v2s_function_name>()
             || node.is<language::binary_v2s_function_name>()) {
    for (auto&& c : node.children) {
      mark_data_kind(*c);
    }
    return node.updateKind(ASTNode::Kind::Scalar);
  } else if (node.is<language::unary_v2v_function_name>()) {
    for (auto&& c : node.children) {
      mark_data_kind(*c);
    }
    return node.updateKind(ASTNode::Kind::Vectorial);
  } else if (node.is<language::plus>() || node.is<language::minus>() || node.is<language::multiply>()
             || node.is<language::divide>()) {
    for (auto&& c : node.children) {
      node.updateKind(mark_data_kind(*c));
    }
    return node.kind();
  } else {
    return ASTNode::Kind::Unknown;
  }
}

std::unique_ptr<IScalarFunction> make_scalar_function(const ASTNode& node);
std::unique_ptr<IVectorFunction> make_vector_function(const ASTNode& node);

auto make_named_unary_s2s_function(const std::string& name, std::unique_ptr<IScalarFunction>&& a) {
  assert(name == "exp");  // FIXME implements the other functions
  return std::make_unique<ExpFunction>(std::move(a));
}

auto make_named_unary_v2v_function(const std::string& name, std::unique_ptr<IVectorFunction>&& a) {
  assert(false);  // FIXME implements the other functions
  return std::make_unique<VectorUndefFunction>(name + "(" + a->string() + ")");
}

auto make_named_unary_v2s_function(const std::string& name, std::unique_ptr<IVectorFunction>&& a) {
  assert(name == "norm2");  // FIXME implements the other functions
  return std::make_unique<ScalarNorm>(std::move(a));
}

auto make_named_binary_v2s_function(const std::string& name,
                                    std::unique_ptr<IVectorFunction>&& a,
                                    std::unique_ptr<IVectorFunction>&& b) {
  assert(name == "dot");  // FIXME implements the other functions
  return std::make_unique<DotProduct>(std::move(a), std::move(b));
}

std::unique_ptr<IScalarFunction> make_scalar_function(const ASTNode& node) {
  if (node.is_root())
    return make_scalar_function(*node.children.front());

  if (node.is<language::unary_s2s_function_name>()) {
    const ASTNode& a = *node.children[0];
    return make_named_unary_s2s_function(node.string(), make_scalar_function(a));
  } else if (node.is<language::unary_v2s_function_name>()) {
    const ASTNode& a = *node.children[0];
    assert(a.kind() == ASTNode::Kind::Vectorial);
    return make_named_unary_v2s_function(node.string(), make_vector_function(a));
  } else if (node.is<language::binary_v2s_function_name>()) {
    const ASTNode& a = *node.children[0];
    const ASTNode& b = *node.children[1];
    assert(a.kind() == b.kind() && b.kind() == ASTNode::Kind::Vectorial);
    assert(node.kind() == ASTNode::Kind::Scalar);
    return make_named_binary_v2s_function(node.string(), make_vector_function(a), make_vector_function(b));
  } else if (node.is<language::scalar_variable>() || node.is<language::scalar_constant>()) {
    return std::make_unique<ScalarValue>(node.content());
  } else if (node.is<language::number>()) {
    return std::make_unique<ScalarNumber>(node.content());
  } else if (node.is<language::indexed_vector_variable>()) {
    const ASTNode& a = *node.children[0];
    const ASTNode& b = *node.children[1];
    assert(b.is<language::index>());
    return std::make_unique<IndexedVectorIdentity>(make_vector_function(a), b.string());
  } else if (node.is<language::prefix_plus>()) {
    const ASTNode& a = *node.children[0];
    return std::make_unique<ScalarPrefixPlus>(make_scalar_function(a));
  } else if (node.is<language::prefix_minus>()) {
    const ASTNode& a = *node.children[0];
    return std::make_unique<ScalarPrefixMinus>(make_scalar_function(a));
  } else if (node.is<language::plus>()) {
    const ASTNode& a = *node.children[0];
    const ASTNode& b = *node.children[1];
    assert(a.kind() == node.kind() && a.kind() == b.kind() && b.kind() == ASTNode::Kind::Scalar);
    return std::make_unique<ScalarAdd>(make_scalar_function(a), make_scalar_function(b));
  } else if (node.is<language::minus>()) {
    const ASTNode& a = *node.children[0];
    const ASTNode& b = *node.children[1];
    assert(a.kind() == node.kind() && a.kind() == b.kind() && b.kind() == ASTNode::Kind::Scalar);
    return std::make_unique<ScalarSub>(make_scalar_function(a), make_scalar_function(b));
  } else if (node.is<language::multiply>()) {
    const ASTNode& a = *node.children[0];
    const ASTNode& b = *node.children[1];
    assert(a.kind() == node.kind() && a.kind() == b.kind() && b.kind() == ASTNode::Kind::Scalar);
    return std::make_unique<ScalarScalarProduct>(make_scalar_function(a), make_scalar_function(b));
  } else if (node.is<language::divide>()) {
    const ASTNode& a = *node.children[0];
    const ASTNode& b = *node.children[1];
    assert(a.kind() == node.kind() && a.kind() == b.kind() && b.kind() == ASTNode::Kind::Scalar);
    return std::make_unique<ScalarScalarDivide>(make_scalar_function(a), make_scalar_function(b));
  } else {
    return std::make_unique<ScalarUndefFunction>(node.name());
  }
}

std::unique_ptr<IVectorFunction> make_vector_function(const ASTNode& node) {
  if (node.is<language::plus>()) {
    const ASTNode& a = *node.children[0];
    const ASTNode& b = *node.children[1];
    assert(a.kind() == node.kind() && a.kind() == b.kind() && b.kind() == ASTNode::Kind::Vectorial);
    return std::make_unique<VectorAdd>(make_vector_function(a), make_vector_function(b));
  } else if (node.is<language::minus>()) {
    const ASTNode& a = *node.children[0];
    const ASTNode& b = *node.children[1];
    assert(a.kind() == node.kind() && a.kind() == b.kind() && b.kind() == ASTNode::Kind::Vectorial);
    return std::make_unique<VectorSub>(make_vector_function(a), make_vector_function(b));
  } else if (node.is<language::prefix_plus>()) {
    const ASTNode& a = *node.children[0];
    return std::make_unique<VectorPrefixPlus>(make_vector_function(a));
  } else if (node.is<language::prefix_minus>()) {
    const ASTNode& a = *node.children[0];
    return std::make_unique<VectorPrefixMinus>(make_vector_function(a));
  } else if (node.is<language::multiply>()) {
    const ASTNode& a = *node.children[0];
    const ASTNode& b = *node.children[1];
    if (a.kind() == ASTNode::Kind::Scalar && b.kind() == ASTNode::Kind::Vectorial) {
      return std::make_unique<ScalarVectorProduct>(make_scalar_function(a), make_vector_function(b));
    } else if (a.kind() == ASTNode::Kind::Vectorial && b.kind() == ASTNode::Kind::Scalar) {
      return std::make_unique<ScalarVectorProduct>(make_scalar_function(b), make_vector_function(a));
    } else {
      return {};
    }
  } else if (node.is<language::vector_variable>()) {
    return std::make_unique<VectorIdentity>(node.content());
  } else if (node.is<language::unary_v2v_function_name>()) {
    const ASTNode& a = *node.children[0];
    return make_named_unary_v2v_function(node.string(), make_vector_function(a));
  } else {
    return std::make_unique<VectorUndefFunction>(node.name());
  }
}

}  // namespace

std::unique_ptr<IScalarFunction> build_function(ASTNode& node) {
  mark_data_kind(node);
  return make_scalar_function(node);
}

ASTNode::Kind ASTNode::updateKind(ASTNode::Kind kind) {
  assert(kind != Kind::Unknown);
  if (m_kind == Kind::Unknown) {
    m_kind = kind;
  } else if (kind != m_kind) {
    assert(kind == Kind::Scalar || m_kind == Kind::Scalar);
    assert(kind == Kind::Vectorial || m_kind == Kind::Vectorial);
    m_kind = Kind::Vectorial;
  }
  return m_kind;
}

std::string IFunction::strHelper(const IFunction& subExpr) const {
  if (subExpr.level() > this->level()) {
    return "(" + subExpr.string() + ")";
  } else {
    return subExpr.string();
  }
}
