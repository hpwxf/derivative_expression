//
// Created by Pascal Havé on 11/06/2020.
//

#include "algebra.hpp"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>

// Pré-déclaration de l'interface
class IS2SFunction;

typedef double ScalarData;

#include <memory>
using IS2SFunctionPtr = std::shared_ptr<const IS2SFunction>;

// Wrapper pour l'interface pour manipuler des objets sans s'occuper de
// l'allocation dynamique
class S2SFunction {
 public:
  S2SFunction(const IS2SFunction* f) : m_f{f} {}
  S2SFunction(IS2SFunctionPtr f) : m_f(std::move(f)) {}
  virtual ~S2SFunction() = default;

 public:
  operator IS2SFunctionPtr() const { return m_f; }

 public:
  ScalarData operator()(const ScalarData x) const;
  S2SFunction diff() const;
  std::ostream& print(std::ostream& o) const;
  int memory_size() const;

 private:
  const IS2SFunctionPtr m_f;
};

// Interface de fonctions
class IS2SFunction {
 public:
  virtual ~IS2SFunction() = default;
  virtual ScalarData eval(ScalarData x) const = 0;
  virtual S2SFunction diff() const = 0;
  virtual std::ostream& print(std::ostream& o) const = 0;
  virtual int memory_size() const = 0;
};

// Identité
class Identity : public IS2SFunction {
 public:
  Identity() = default;
  ScalarData eval(ScalarData x) const final { return x; }
  S2SFunction diff() const final;
  std::ostream& print(std::ostream& o) const final {
    o << 'x';
    return o;
  }
  int memory_size() const final { return sizeof(*this); }
};

// Constante
class Constant : public IS2SFunction {
 public:
  explicit Constant(ScalarData v) : m_v(v) {}
  ScalarData eval(ScalarData x) const final { return m_v; }
  S2SFunction diff() const final;
  std::ostream& print(std::ostream& o) const final {
    o << m_v;
    return o;
  }
  int memory_size() const final { return sizeof(*this); }

 private:
  const ScalarData m_v;
};

// Sinus
class Sinus : public IS2SFunction {
 public:
  Sinus(IS2SFunctionPtr f) : m_f(std::move(f)) {}
  ScalarData eval(ScalarData x) const final { return std::sin(m_f->eval(x)); }
  S2SFunction diff() const final;
  std::ostream& print(std::ostream& o) const final {
    o << "sin(";
    m_f->print(o);
    o << ')';
    return o;
  }
  int memory_size() const final { return sizeof(*this) + m_f->memory_size(); }

 private:
  const IS2SFunctionPtr m_f;
};

// Cosinus
class Cosinus : public IS2SFunction {
 public:
  Cosinus(IS2SFunctionPtr f) : m_f(std::move(f)) {}
  ScalarData eval(ScalarData x) const final { return std::cos(m_f->eval(x)); }
  S2SFunction diff() const final;
  std::ostream& print(std::ostream& o) const final {
    o << "cos(";
    m_f->print(o);
    o << ')';
    return o;
  }
  int memory_size() const final { return sizeof(*this) + m_f->memory_size(); }

 private:
  const IS2SFunctionPtr m_f;
};

// Addition
class Add : public IS2SFunction {
 public:
  Add(IS2SFunctionPtr f, IS2SFunctionPtr g) : m_f(std::move(f)), m_g(std::move(g)) {}
  ScalarData eval(ScalarData x) const final { return m_f->eval(x) + m_g->eval(x); }
  S2SFunction diff() const final;
  std::ostream& print(std::ostream& o) const final {
    o << '(';
    m_f->print(o);
    o << ") + (";
    m_g->print(o);
    o << ')';
    return o;
  }
  int memory_size() const final { return sizeof(*this) + m_f->memory_size() + m_g->memory_size(); }

 private:
  const IS2SFunctionPtr m_f, m_g;
};

// Multiplication
class Mult : public IS2SFunction {
 public:
  Mult(IS2SFunctionPtr f, IS2SFunctionPtr g) : m_f(std::move(f)), m_g(std::move(g)) {}
  ScalarData eval(ScalarData x) const final { return m_f->eval(x) * m_g->eval(x); }
  S2SFunction diff() const final;
  std::ostream& print(std::ostream& o) const final {
    o << '(';
    m_f->print(o);
    o << ") * (";
    m_g->print(o);
    o << ')';
    return o;
  }
  int memory_size() const final { return sizeof(*this) + m_f->memory_size() + m_g->memory_size(); }

 private:
  const IS2SFunctionPtr m_f, m_g;
};

// Inverse
class Inverse : public IS2SFunction {
 public:
  Inverse(IS2SFunctionPtr f) : m_f(std::move(f)) {}
  ScalarData eval(ScalarData x) const final { return 1. / m_f->eval(x); }
  S2SFunction diff() const final;
  std::ostream& print(std::ostream& o) const final {
    o << "inv(";
    m_f->print(o);
    o << ")";
    return o;
  }
  int memory_size() const final { return sizeof(*this) + m_f->memory_size(); }

 private:
  const IS2SFunctionPtr m_f;
};

// Puissance
class Pow : public IS2SFunction {
 public:
  Pow(IS2SFunctionPtr f, const int n) : m_f(std::move(f)), m_n(n) {
    if (n == 0)
      throw std::exception();
  }
  ScalarData eval(ScalarData x) const final { return std::pow(m_f->eval(x), m_n); }
  S2SFunction diff() const final;
  // std::ostream & print(std::ostream & o) const { o << '('; m_f->print(o); o
  // << ") ^" << m_n; return o; }
  std::ostream& print(std::ostream& o) const final {
    o << "std::pow(";
    m_f->print(o);
    o << "," << m_n << ")";
    return o;
  }
  int memory_size() const final { return sizeof(*this) + m_f->memory_size(); }

 private:
  const IS2SFunctionPtr m_f;
  const int m_n;
};

// Impl�mentation de l'op�rateur d'�valuation
ScalarData S2SFunction::operator()(const ScalarData x) const {
  return m_f->eval(x);
}
S2SFunction S2SFunction::diff() const {
  return m_f->diff();
}
std::ostream& S2SFunction::print(std::ostream& o) const {
  return m_f->print(o);
}
int S2SFunction::memory_size() const {
  return sizeof(*this) + m_f->memory_size();
}

// Quelques fonctions pour l'interface utilisateur
S2SFunction sin(const S2SFunction& f) {
  return S2SFunction(new Sinus(f));
}
S2SFunction cos(const S2SFunction& f) {
  return S2SFunction(new Cosinus(f));
}
// Des opérateurs primaires
S2SFunction operator+(const S2SFunction& f, const S2SFunction& g) {
  return new Add(f, g);
}
S2SFunction operator*(const S2SFunction& f, const S2SFunction& g) {
  return new Mult(f, g);
}
S2SFunction operator/(const S2SFunction& f, const S2SFunction& g) {
  return f * S2SFunction(new Inverse(g));
}
// Autres opérateurs associés
S2SFunction operator+(const S2SFunction& f, const ScalarData& a) {
  return new Add(f, S2SFunction(new Constant(a)));
}
S2SFunction operator*(const ScalarData& a, const S2SFunction& f) {
  return new Mult(S2SFunction(new Constant(a)), f);
}
S2SFunction operator/(const ScalarData& a, const S2SFunction& f) {
  return a * S2SFunction(new Inverse(f));
}
S2SFunction operator-(const S2SFunction& f, const ScalarData& a) {
  return new Add(f, S2SFunction(new Constant(-a)));
}
// Des opérations déduits
S2SFunction operator+(const ScalarData& a, const S2SFunction& f) {
  return f + a;
}
S2SFunction operator*(const S2SFunction& f, const ScalarData& a) {
  return a * f;
}
S2SFunction operator/(const S2SFunction& f, const ScalarData& a) {
  return f * (1. / a);
}
S2SFunction operator-(const ScalarData& a, const S2SFunction& f) {
  return f - a;
}
S2SFunction operator-(const S2SFunction& f) {
  return -1 * f;
}
S2SFunction operator-(const S2SFunction& f, const S2SFunction& g) {
  return f + (-g);
}
// Exemple d'optimisation sur l'opérateur ^
S2SFunction operator^(const S2SFunction& f,
                      const int& n) {  // ATTN: opérateur de basse priorité
  if (n == 0)
    return new Constant(1);
  else if (n == 1)
    return f;
  else
    return new Pow(f, n);
}

// Les calculs de dérivées symboliques
S2SFunction Identity::diff() const {
  return new Constant(1);
}
S2SFunction Constant::diff() const {
  return new Constant(0);
}
S2SFunction Sinus::diff() const {
  return cos(m_f) * m_f->diff();
}
S2SFunction Cosinus::diff() const {
  return -sin(m_f) * m_f->diff();
}
S2SFunction Add::diff() const {
  return m_f->diff() + m_g->diff();
}
S2SFunction Mult::diff() const {
  return m_f->diff() * S2SFunction(m_g) + S2SFunction(m_f) * m_g->diff();
}
S2SFunction Inverse::diff() const {
  return -m_f->diff() / S2SFunction(new Pow(m_f, 2));
}
S2SFunction Pow::diff() const {
  // m_n != 0 par construction
  if (m_n == 1)
    return m_f->diff();
  else
    return m_n * (S2SFunction(m_f) ^ (m_n - 1)) * m_f->diff();
}

// Op�rateur de d�rivation
S2SFunction d(const S2SFunction& f) {
  return f.diff();
}

// Affichage d'une expression
std::ostream& operator<<(std::ostream& o, const S2SFunction& f) {
  return f.print(o);
}

void plot(const S2SFunction& f, const char* filename) {
  std::ofstream o(filename);
  const ScalarData xmin = -1;
  const ScalarData xmax = +1;
  const int n = (1u << 16u);

  ScalarData sum = 0;
  const ScalarData dx = (xmax - xmin) / n;
  for (int i = 0; i <= n; ++i) {
    const ScalarData x = xmin + i * dx;
    sum += f(x);
    // o << x << ' ' << f(x) << '\n';
  }
  std::cout << "Sum is " << sum << std::endl;
  // auto-close when out of scope
}

int main() {
  S2SFunction x(new Identity());

  S2SFunction g = sin(x);
  S2SFunction dg = d(g);
  std::cout << "g(1) = " << std::setw(8) << g(1) << ";\t g'(1) = " << std::setw(8) << dg(1) << std::endl;
  S2SFunction h = x ^ 4;
  std::cout << "h(2) = " << std::setw(8) << h(2) << ";\t h'(2) = " << std::setw(8) << d(d(h))(2.) << std::endl;
  // S2SFunction f = (sin(8*x-1)^3.) / cos(x^2.);
  S2SFunction f = (((2 * x + 1) ^ 4) - ((x - 1) ^ 3) / (1 + x + ((x / 2.) ^ 3)));
  std::cout << "f(2) = " << std::setw(8) << f(2) << ";\t f'(2) = " << std::setw(8) << d(f)(2.) << std::endl;
  std::cout << "  f  is " << f << std::endl;
  std::cout << "d(f) is " << d(f) << std::endl;

  std::cout << "Sizeofs : "
            << "\n\t  x=" << (x).memory_size() << "\n\t  g=" << (g).memory_size() << "\n\t dg=" << (dg).memory_size()
            << "\n\t  h=" << (h).memory_size() << "\n\tddh=" << (d(d(h))).memory_size()
            << "\n\t  f=" << (f).memory_size() << "\n\t df=" << (d(f)).memory_size() << "\n";

  plot(f, "f.dat");
  plot(d(f), "df.dat");
};
