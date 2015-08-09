// Copyright 2015, Georg Sauthoff <mail@georg.so>

/* {{{ LGPLv3

    This file is part of libgrammar.

    libgrammar is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libgrammar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with libgrammar.  If not, see <http://www.gnu.org/licenses/>.

}}} */
#include "grammar.hh"

#include <memory>

using namespace std;

namespace grammar {

  namespace asn1 {
    Printer::Printer(std::ostream &o)
      :
        o_(o)
    {
    }
    std::ostream &Printer::o()
    {
      return o_;
    }

    Printer &operator<<(Printer &o, const Grammar &g)
    {
      for (auto nt : g.nts()) {
        o << *nt;
        o.o() << "\n\n";
      }
      return o;
    }
    Printer &operator<<(Printer &o, const Symbol::Base &symbol)
    {
      o.o() << symbol.name();
      return o;
    }
    Printer &operator<<(Printer &o, const Coordinates &coord)
    {
      if (coord.initialized()) {
        o.o() << '[';
        switch (coord.klasse()) {
          case 0:
            o.o() << "UNIVERSAL ";
            break;
          case 1:
            o.o() << "APPLICATION ";
            break;
          case 2:
            // if no class is given, it is context dependent
            //o.o() << "CONTEXT";
            break;
          case 3:
            o.o() << "PRIVATE ";
            break;
        }
        o.o() << coord.tag() << ']';
      }
      return o;
    }
    Printer &operator<<(Printer &o, const Symbol::NT &nt)
    {
      const Symbol::Base &b = nt;
      o << b;
      o.o() << " ::= ";
      o << nt.coord();
      if (nt.coord().initialized())
        o.o() << ' ';
      o << nt.rule();
      return o;
    }
    Printer &operator<<(Printer &o, const Rule::Base &rule)
    {
      rule.accept(o);
      return o;
    }
    Printer &operator<<(Printer &o, const Rule::Link &rule)
    {
      if (rule.refs().empty())
        return o;
      o.o() << rule.refs().front()->symbol_name();
      if (!rule.refs().front()->constraints().empty()) {
        o.o() << " (";
        auto i = rule.refs().front()->constraints().begin();
        auto e = rule.refs().front()->constraints().end();
        o << **i;
        ++i;
        for (; i != e; ++i) {
          o.o() << ", ";
          o << **i;
        }
        o.o() << ")";
      }
      return o;
    }
    static Printer &print_record(Printer &o, const Rule::Base &rule,
        const char *name)
    {
      o.o() << name << "\n{\n";
      auto i = rule.refs().begin();
      if (i != rule.refs().end()) {
        o.o() << "  ";
        o << **i;
        ++i;
      }
      for (; i != rule.refs().end(); ++i) {
        o.o() << ",\n  ";
        o << **i;
      }
      if (rule.extensible())
        o.o() << ",\n  ...";
      o.o() << "\n}";
      return o;
    }
    Printer &operator<<(Printer &o, const Rule::Choice &rule)
    {
      return print_record(o, rule, "CHOICE");
    }
    Printer &operator<<(Printer &o, const Rule::Sequence &rule)
    {
      return print_record(o, rule, "SEQUENCE");
    }
    Printer &operator<<(Printer &o, const Rule::All &rule)
    {
      return print_record(o, rule, "SET");
    }

    Printer &operator<<(Printer &o, const Reference &ref)
    {
      if (!ref.name().empty()) {
        o.o() << ref.name() << ' ';
      }
      o << ref.coord();
      if (ref.coord().initialized())
        o.o() << ' ';
      o.o() << ref.symbol_name();
      for (auto &constraint : ref.constraints()) {
        o.o() << ' '; o << *constraint;
      }
      if (ref.optional()) {
        o.o() << ' ' << "OPTIONAL";
      }
      return o;
    }
    static Printer &print_of(Printer &o, const Rule::Base &rule,
        const char *name)
    {
      o.o() << name << " OF ";
      if (rule.refs().empty())
        return o;
      o << *rule.refs().front();
      return o;
    }
    Printer &operator<<(Printer &o, const Rule::List &rule)
    {
      return print_of(o, rule, "SEQUENCE");
    }
    Printer &operator<<(Printer &o, const Rule::Set &rule)
    {
      return print_of(o, rule, "SET");
    }
    Printer &operator<<(Printer &o, const Constraint::Base &c)
    {
      c.accept(o);
      return o;
    }
    Printer &operator<<(Printer &o, const Constraint::Size &c)
    {
      o.o() << "SIZE(" << c.range().first;
      if (c.range().first != c.range().second)
        o.o() << ".." << c.range().second;
      o.o() << ')';
      return o;
    }
    Printer &operator<<(Printer &o, const Constraint::Domain &c)
    {
      o.o() << '(';
      if (c.has_min())
        o.o() << c.min();
      if (c.has_max())
        o.o() << ".." << c.max();
      o.o()  << ')';
      return o;
    }

    void Printer::visit(const Rule::Link &r)
    {
      (*this) << r;
    }
    void Printer::visit(const Rule::Choice &r)
    {
      (*this) << r;
    }
    void Printer::visit(const Rule::Sequence &r)
    {
      (*this) << r;
    }
    void Printer::visit(const Rule::All &r)
    {
      (*this) << r;
    }
    void Printer::visit(const Rule::List &r)
    {
      (*this) << r;
    }
    void Printer::visit(const Rule::Set &r)
    {
      (*this) << r;
    }
    void Printer::visit(const Constraint::Size &r)
    {
      (*this) << r;
    }
    void Printer::visit(const Constraint::Domain &r)
    {
      (*this) << r;
    }


    void add_terminals(Grammar &g)
    {
      // EOC
      g.push(make_unique<Symbol::Terminal>("BOOLEAN", Coordinates(1, 0)));
      g.push(make_unique<Symbol::Terminal>("INTEGER", Coordinates(2, 0)));
      g.push(make_unique<Symbol::Terminal>("BIT STRING", Coordinates(3, 0)));
      g.push(make_unique<Symbol::Terminal>("OCTET STRING", Coordinates(4, 0)));
      g.push(make_unique<Symbol::Terminal>("NULL", Coordinates(5, 0)));
      g.push(make_unique<Symbol::Terminal>("OBJECT IDENTIFIER", Coordinates(6, 0)));
      g.push(make_unique<Symbol::Terminal>("ObjectDescriptor", Coordinates(7, 0)));
      g.push(make_unique<Symbol::Terminal>("EXTERNAL", Coordinates(8, 0)));
      g.push(make_unique<Symbol::Terminal>("REAL", Coordinates(9, 0)));
      g.push(make_unique<Symbol::Terminal>("ENUMERATED", Coordinates(10, 0)));
      g.push(make_unique<Symbol::Terminal>("EMBEDDED PDV", Coordinates(11, 0)));
      g.push(make_unique<Symbol::Terminal>("UTF8String", Coordinates(12, 0)));
      g.push(make_unique<Symbol::Terminal>("RELATIVE-OID", Coordinates(13, 0)));
      g.push(make_unique<Symbol::Terminal>("TIME", Coordinates(14, 0)));
      // RESERVED_15
      // and SEQUENCE OF
      g.push(make_unique<Symbol::Terminal>("SEQUENCE", Coordinates(16, 0)));
      // and SET OF
      g.push(make_unique<Symbol::Terminal>("SET", Coordinates(17, 0)));
      g.push(make_unique<Symbol::Terminal>("NumericString", Coordinates(18, 0)));
      g.push(make_unique<Symbol::Terminal>("PrintableString", Coordinates(19, 0)));
      g.push(make_unique<Symbol::Terminal>("TeletexString", Coordinates(20, 0)));
      g.push(make_unique<Symbol::Terminal>("VideotexString", Coordinates(21, 0)));
      g.push(make_unique<Symbol::Terminal>("IA5String", Coordinates(22, 0)));
      g.push(make_unique<Symbol::Terminal>("UTCTime", Coordinates(23, 0)));
      g.push(make_unique<Symbol::Terminal>("GeneralizedTime", Coordinates(24, 0)));
      g.push(make_unique<Symbol::Terminal>("GraphicString", Coordinates(25, 0)));
      g.push(make_unique<Symbol::Terminal>("VisibleString", Coordinates(26, 0)));
      g.push(make_unique<Symbol::Terminal>("GeneralString", Coordinates(27, 0)));
      g.push(make_unique<Symbol::Terminal>("UniversalString", Coordinates(28, 0)));
      g.push(make_unique<Symbol::Terminal>("CHARACTER STRING", Coordinates(29, 0)));
      g.push(make_unique<Symbol::Terminal>("BMPString", Coordinates(30, 0)));
      g.push(make_unique<Symbol::Terminal>("DATE", Coordinates(31, 0)));
      g.push(make_unique<Symbol::Terminal>("TIME-OF-DAY", Coordinates(32, 0)));
      g.push(make_unique<Symbol::Terminal>("DATE-TIME", Coordinates(33, 0)));
      g.push(make_unique<Symbol::Terminal>("DURATION", Coordinates(34, 0)));
      g.push(make_unique<Symbol::Terminal>("OID-IRI", Coordinates(35, 0)));
      g.push(make_unique<Symbol::Terminal>("RELATIVE-OID-IRI", Coordinates(36, 0)));
    }

  }

}
