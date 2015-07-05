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
#include "rng.hh"

#include <boost/algorithm/string/replace.hpp>

using namespace std;

namespace grammar {

  namespace xml {

    namespace rng {


      static void print_axiom(Printer &o, const Grammar &g)
      {
        if (g.axiom().rule().type() == Rule::CHOICE
            && !g.axiom().coord().initialized()) {
          o.o() << "\n  <start>\n"
            "      <ref name='" << g.axiom().name() << "'/>\n"
            "  </start>\n\n";
        } else {
          o.o() << "\n  <start>\n"
            "    <element name='" << g.axiom().name() << "'>\n"
            "      <ref name='" << g.axiom().name() << "'/>\n"
            "    </element>\n"
            "  </start>\n\n";
        }
      }

      void Printer::visit(const Grammar &g)
      {
        o() << "<?xml version='1.0'?>\n";
        print_comment();
o() << R"(<grammar datatypeLibrary='http://www.w3.org/2001/XMLSchema-datatypes'
)";
        if (!target_namespace().empty())
          o() << "         ns='" << target_namespace() << "'\n";
        o() << R"(         xmlns='http://relaxng.org/ns/structure/1.0'>
)";

        print_axiom(*this, g);
        for (auto &nt : g.nts()) {
          visit(*nt);
          o() << "\n\n";
        }
        for (auto &terminal : g.terminals()) {
          visit(*terminal);
          o() << "\n\n";
        }
        o() << "</grammar>";
      }

      static void print_terminal_type_start(Printer &o,
          const std::string &name)
      {
        o.o() << "<data type='";
        if (name == "INTEGER") {
          o.o() << "integer";
        } else if (name == "BOOLEAN") {
          o.o() << "boolean";
        } else {
          o.o() << "string";
        }
        o.o() << "'>";
      }

      void Printer::visit(const Symbol::NT &nt)
      {
        auto &eff_symbol = grammar::effective_symbol(nt.rule().refs().front());
        if (nt.rule().type() == Rule::LINK
            && dynamic_cast<const Symbol::Terminal*>(&eff_symbol)) {
          o() << "  <define name='" << nt.name() << "'>\n";
          auto &symbol = grammar::effective_symbol(nt.rule().refs().front());

          if (symbol.name() == "INTEGER"
              && !nt.rule().refs().front().constraints().empty()
              && dynamic_cast<const Constraint::Enum*>(
                nt.rule().refs().front().constraints().front().get())
              ) {
            // at least libxml2 does not seem to support the xsd enumeration
            // facet
            o() << "    <choice>\n";
            auto &e = *dynamic_cast<const Constraint::Enum*>(
                nt.rule().refs().front().constraints().front().get());
            for (auto &v : e.values())
              o() << "      <value>" << v << "</value>\n";
            o() << "    </choice>\n";
          } else {
            o() << "    ";
            print_terminal_type_start(*this, symbol.name());
            o() << '\n';
            symbol_name_ = nt.rule().refs().front().symbol_name();
            for (auto &constraint : nt.rule().refs().front().constraints()) {
              (*this) << *constraint;
              o() << '\n';
            }
            o() << "    </data>\n";
          }
          o() << "  </define>";
        } else {
          o() << "  <define name='" << nt.name() << "'>\n";
          (*this) << nt.rule();
          o() << "\n  </define>";

        }
      }

      void Printer::visit(const Constraint::Size &c)
      {
        size_t k = 1u;
        if (c.source() == Constraint::Source::ASN1
            && symbol_name_ == "BCDString")
          k = 2u;
        o() << "      <param name='minLength'>" << k*c.range().first
          << "</param>\n";
        o() << "      <param name='maxLength'>" << k*c.range().second
          << "</param>";
      }
      void Printer::visit(const Constraint::Enum &c)
      {
        for (auto &v : c.values()) {
          o() << "      <param name='enumeration'>" << v << "</param>\n";
        }
      }
      void Printer::visit(const Constraint::Pattern &c)
      {
        o() << "      <param name='pattern'>" << c.str() << "</param>";
      }
      void Printer::visit(const Constraint::Domain &c)
      {
        if (c.has_min())
          o() << "      <param name='minInclusive'>" << c.min() << "</param>\n";
        if (c.has_max())
          o() << "      <param name='maxInclusive'>" << c.max() << "</param>";
      }

      void Printer::visit(const Symbol::Terminal &terminal)
      {
        o() << "  <define name='" << boost::replace_all_copy(terminal.name(), " ", "_") << "'>\n";
        o() << "    ";
        print_terminal_type_start(*this, terminal.name());
        o() << '\n';
        o() << "    </data>\n";
        o() << "  </define>";
      }
      static void print_name_type(Printer &o, const Reference &ref)
      {
        if (ref.optional())
          o.o() << "    <optional>\n";

        o.o() << "    <element name='";
        //if (ref.name().empty())
          o.o() << boost::replace_all_copy(ref.symbol_name(), " ", "_");
        //else
        //  o.o() << ref.name();
        o.o() << "'>\n";

        o.o() << "      <ref name='";

        auto &symbol = ref.symbol();
        auto &eff_symbol = grammar::effective_symbol(ref);
        string symbol_name;
        if (dynamic_cast<const Symbol::Terminal*>(&eff_symbol))
          symbol_name = symbol.name();
        else
          symbol_name = eff_symbol.name();

        o.o()  << boost::replace_all_copy(symbol_name, " ", "_");

        o.o() << "'/>\n";

        o.o() << "    </element>";

        if (ref.optional())
          o.o() << "\n    </optional>";
      }
      static void print_sequence(Printer &o, const Rule::Base &rule)
      {
        auto i = rule.refs().begin();
        auto e = rule.refs().end();
        if (i != e)
          print_name_type(o, *i++);
        for (; i != e; i++) {
          o.o() << '\n';
          print_name_type(o, *i);
        }
      }
      void Printer::visit(const Rule::Link &rule)
      {
        print_sequence(*this, rule);
      }
      void Printer::visit(const Rule::Sequence &rule)
      {
        print_sequence(*this, rule);
      }
      static void print_list(Printer &o, const Rule::Base &rule)
      {
        auto &eff_symbol = grammar::effective_symbol(rule.refs().front());
        if (dynamic_cast<const Symbol::NT*>(&eff_symbol) &&
            dynamic_cast<const Symbol::NT*>(&eff_symbol)->rule().type()
            == Rule::CHOICE && !eff_symbol.coord().initialized()) {
          o.o() << "    <oneOrMore>\n";
          o.o() << "      <ref name='"
            << rule.refs().front().symbol_name() << "'/>\n";
          o.o() << "    </oneOrMore>";
        } else {
          o.o() << "    <oneOrMore>\n";
          print_name_type(o, rule.refs().front());
          o.o() << '\n';
          o.o() << "    </oneOrMore>";
        }
      }
      void Printer::visit(const Rule::List &rule)
      {
        print_list(*this, rule);
      }
      void Printer::visit(const Rule::Set &rule)
      {
        print_list(*this, rule);
      }
      static void print_choice_all(Printer &o, const Rule::Base &rule,
          const char *name)
      {
        o.o() << "    <" << name << ">\n";
        for (auto &ref : rule.refs()) {
          print_name_type(o, ref);
          o.o() << '\n';
        }
        o.o() << "    </" << name << ">"
          ;
      }
      void Printer::visit(const Rule::Choice &rule)
      {
        print_choice_all(*this, rule, "choice");
      }
      void Printer::visit(const Rule::All &rule)
      {
        print_choice_all(*this, rule, "interleave");
      }


    }

  }

}
