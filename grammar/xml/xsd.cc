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
#include "xsd.hh"

#include <boost/algorithm/string/replace.hpp>
#include <ixxx/util.hh>

using namespace std;

namespace grammar {

  namespace xml {

    namespace xsd {

      void Printer::print_key_defs(const std::string &nt)
      {
        auto i = nt_key_defs_.find(nt);
        if (i == nt_key_defs_.end())
            return;
        o() << i->second;
      }

      static void print_axiom(Printer &o, const Grammar &g)
      {
        if (g.axiom().rule().type() == Rule::CHOICE
            && !g.axiom().coord().initialized()) {
          for (auto &ref : g.axiom().rule().refs()) {
            o.o() << "  <xs:element name='" << ref->symbol_name()
                  << "' type='" << ref->symbol_name() << "'>\n";
            o.print_key_defs(ref->symbol_name());
            o.o() << "  </xs:element>\n";
          }
        } else {
          o.o() << "  <xs:element name='" << g.axiom().name()
                << "' type='" << g.axiom().name() << "'>\n";
          o.print_key_defs(g.axiom().name());
          o.o() << "  </xs:element>\n";
        }
      }

      void Printer::visit(const Grammar &g)
      {
        o() << "<?xml version='1.0' encoding='UTF-8'?>\n";
        print_comment();
o() << R"(<xs:schema
  xmlns:xs='http://www.w3.org/2001/XMLSchema'
)";
        if (!target_namespace().empty()) {
          o() << R"(  targetNamespace=')" << target_namespace() << R"('
            xmlns=')" << target_namespace() << R"('
            xmlns:ns=')" << target_namespace() << "'\n";
        }
        o() << "  elementFormDefault='qualified'>\n";
        print_axiom(*this, g);
        for (auto &nt : g.nts()) {
          visit(*nt);
          o() << "\n\n";
        }
        for (auto &terminal : g.terminals()) {
          visit(*terminal);
          o() << "\n\n";
        }
        o() << "</xs:schema>\n";
      }

      void Printer::visit(const Symbol::NT &nt)
      {
        nt_name_ = nt.name();
        auto &eff_symbol = grammar::effective_symbol(*nt.rule().refs().front());
        if (nt.rule().type() == Rule::LINK
            && dynamic_cast<const Symbol::Terminal*>(&eff_symbol)) {
          o() << "  <xs:simpleType name='";
          o()  << nt.name();
          if (any_attribute_)
            o() << "___base";
          o()  << "'>\n";
          auto &symbol = grammar::effective_symbol(*nt.rule().refs().front());
          o() << "    <xs:restriction base='";;
          o() << boost::replace_all_copy(symbol.name(), " ", "_");
          o() << "'>\n";
          symbol_name_ = nt.rule().refs().front()->symbol_name();
          for (auto &constraint : nt.rule().refs().front()->constraints()) {
            (*this) << *constraint;
            o() << '\n';
          }
          o() << "    </xs:restriction>\n";
          o() << "  </xs:simpleType>";

          if (any_attribute_) {
            o() << "\n  <xs:complexType name='" << nt.name() << "'>\n"
                  "    <xs:simpleContent>\n"
                  "      <xs:extension base='" << nt.name() << "___base'>\n"
                  "        <xs:anyAttribute processContents='lax'/>\n"
                  "      </xs:extension>\n"
                  "    </xs:simpleContent>\n"
                  "  </xs:complexType>\n";
          }
        } else if (nt.rule().type() == Rule::CHOICE
            && !nt.coord().initialized()) {
          o() << "  <xs:group name='" << nt.name() << "'>\n";
          (*this) << nt.rule();
          o() << '\n';
          o() << "  </xs:group>";
        } else {
          o() << "  <xs:complexType name='" << nt.name() << "'>\n";
          (*this) << nt.rule();
          o() << '\n';
          if (any_attribute_) {
            o() << "    <xs:anyAttribute processContents='lax'/>\n";
          }
          o() << "  </xs:complexType>";
        }
      }

      void Printer::visit(const Constraint::Size &c)
      {
        size_t k = 1u;
        if (c.source() == Constraint::Source::ASN1
            && symbol_name_ == "BCDString")
          k = 2u;
        o() << "      <xs:minLength value='" << k*c.range().first << "'/>\n";
        o() << "      <xs:maxLength value='" << k*c.range().second << "'/>";
      }
      void Printer::visit(const Constraint::Enum &c)
      {
        for (auto &v : c.values()) {
          o() << "      <xs:enumeration value='" << v << "'/>\n";
        }
      }
      void Printer::visit(const Constraint::Pattern &c)
      {
        o() << "      <xs:pattern value='" << c.str() << "'/>";
      }
      void Printer::visit(const Constraint::Domain &c)
      {
        if (c.has_min())
          o() << "      <xs:minInclusive value='" << c.min() << "'/>\n";
        if (c.has_max())
          o() << "      <xs:maxInclusive value='" << c.max() << "'/>";
      }

      void Printer::visit(const Symbol::Terminal &terminal)
      {
        o() << "  <xs:simpleType name='";
        o() << boost::replace_all_copy(terminal.name(), " ", "_");
        o() << "'>\n";
        o() << "    <xs:restriction base='";
        if (terminal.name() == "INTEGER") {
          o() << "xs:integer";
        } else if (terminal.name() == "BOOLEAN") {
          o() << "xs:boolean";
        } else {
          o() << "xs:string";
        }
        o() << "'>\n";
        o() << "    </xs:restriction>\n";
        o() << "  </xs:simpleType>";
      }

      /* There 2 possibilities how to name the elements
       * when there are named type references:
       *
       * - the name
       * - the type name
       *
       * With asn.1 grammars that are all-implicit and
       * have tag-less named references the type name is more convenient,
       * since it simplies the mapping XML -> BER.
       */
      static void print_name_type(Printer &o, const Reference &ref)
      {
        o.o() << "name='";
        //if (ref.name().empty())
          o.o() << boost::replace_all_copy(ref.symbol_name(), " ", "_");
        //else
        //  o.o() << ref.name();
        o.o() << "' type='";
        auto &symbol = ref.symbol();
        auto &eff_symbol = grammar::effective_symbol(ref);
        string symbol_name;
        if (dynamic_cast<const Symbol::Terminal*>(&eff_symbol))
          symbol_name = symbol.name();
        else
          symbol_name = eff_symbol.name();
        o.o()  << boost::replace_all_copy(symbol_name, " ", "_");
        o.o()  << "'";
        if (ref.optional())
          o.o() << " minOccurs='0'";
      }
      static void print_sequence(Printer &o, const Rule::Base &rule)
      {
        o.o() << "    <xs:sequence>\n";
        for (auto &ref : rule.refs()) {
          o.o() << "      <xs:element ";
          print_name_type(o, *ref);
          o.o() << "/>\n";
        }
        o.o() << "    </xs:sequence>";
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
        o.o() << "    <xs:sequence>\n";

        auto &eff_symbol = grammar::effective_symbol(*rule.refs().front());
        if (dynamic_cast<const Symbol::NT*>(&eff_symbol) &&
            dynamic_cast<const Symbol::NT*>(&eff_symbol)->rule().type()
            == Rule::CHOICE && !eff_symbol.coord().initialized()) {
          o.o() << "      <xs:group ref='" << eff_symbol.name()
            << "' maxOccurs='unbounded'/>\n";
        } else {


          o.o() << "      <xs:element ";
          print_name_type(o, *rule.refs().front());
          o.o() << " maxOccurs='unbounded'";
          o.o() << "/>\n";

        }


        o.o() << "    </xs:sequence>";
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
        o.o() << "      <xs:" << name << ">\n";
        for (auto &ref : rule.refs()) {
          o.o() << "      <xs:element ";
          print_name_type(o, *ref);
          o.o() << "/>\n";
        }
        o.o() << "      </xs:" << name << ">";
      }
      void Printer::visit(const Rule::Choice &rule)
      {
        print_choice_all(*this, rule, "choice");
      }
      void Printer::visit(const Rule::All &rule)
      {
        print_choice_all(*this, rule, "all");
      }

      void Printer::read_key_def(const std::string &nt,
          const std::string &filename)
      {
        auto f = ixxx::util::mmap_file(filename);
        nt_key_defs_[nt] = string(f.s_begin(), f.s_end());
      }

      void Printer::set_any_attribute(bool b)
      {
        any_attribute_ = b;
      }

    }

  }

}
