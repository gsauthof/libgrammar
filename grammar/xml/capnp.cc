// Copyright 2017, Georg Sauthoff <mail@georg.so>

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
#include "capnp.hh"

#include <boost/algorithm/string.hpp>

using namespace std;

namespace grammar {

  namespace xml {

    namespace capnp {


      void Printer::visit(const Grammar &g)
      {
        for (auto &nt : g.nts()) {
          visit(*nt);
        }
      }

      void Printer::visit(const Symbol::NT &nt)
      {
        if (nt.rule().type() == Rule::LINK)
          return;
        if (nt.rule().type() == Rule::LIST)
          return;
        o() << "  struct " << nt.name() << " {\n";
        (*this) << nt.rule();
        o() << "\n  }\n\n";
      }

      static void print_type(Printer &o, const Reference &ref)
      {
        auto &eff_symbol = grammar::effective_symbol(ref);
        const Symbol::NT *nt = dynamic_cast<const Symbol::NT*>(&eff_symbol);
        if (nt) {
          string s = boost::erase_all_copy(eff_symbol.name(), " ");
          o.o() << s;
        } else {
          if (eff_symbol.name() == "INTEGER")
            o.o() << "Int64";
          else if (eff_symbol.name() == "BOOLEAN")
            o.o() << "Bool";
          else
            o.o() << "Data";
        }
      }
      static void print_name_type(Printer &o, const Reference &ref,
          unsigned &cnt, const string &indent)
      {
        string name = boost::replace_all_copy(ref.name(), " ", "_");
        o.o() << indent << name;
        if (!ref.optional()) {
          o.o() << " @" << cnt << " ";
          ++cnt;
        }
        o.o() << ": ";
        if (ref.optional()) {
          o.o() << "union {\n";
          o.o() << indent << "  some @" << cnt << " : ";
          ++cnt;
        }

        auto &eff_symbol = grammar::effective_symbol(ref);
        const Symbol::NT *nt = dynamic_cast<const Symbol::NT*>(&eff_symbol);
        if (nt && nt->rule().type() == Rule::LIST) {
          o.o() << "List(";
          print_type(o, *nt->rule().refs().front());
          o.o() << ')';
        } else {
          print_type(o, ref);
        }
        o.o() << ";";

        if (ref.optional()) {
          o.o() << "\n" << indent << "  none @" << cnt << " : Void;\n";
          ++cnt;
          o.o() << "    }";
        }
      }
      static void print_sequence(Printer &o, const Rule::Base &rule)
      {
        unsigned cnt = 0;
        static string indent(4, ' ');
        for (auto &x : rule.refs()) {
          print_name_type(o, *x, cnt, indent);
          o.o() << "\n";
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
      void Printer::visit(const Rule::Set &rule)
      {
        print_sequence(*this, rule);
      }
      static void print_choice_all(Printer &o, const Rule::Base &rule)
      {
        o.o() << "    union {\n";
        unsigned cnt = 0;
        static string indent(6, ' ');
        for (auto &ref : rule.refs()) {
          print_name_type(o, *ref, cnt, indent);
          o.o() << '\n';
        }
        o.o() << "    }";
      }
      void Printer::visit(const Rule::Choice &rule)
      {
        print_choice_all(*this, rule);
      }
      void Printer::visit(const Rule::All &rule)
      {
        print_choice_all(*this, rule);
      }


    }

  }

}
