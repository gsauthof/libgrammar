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
#include "dot.hh"

#include <grammar/tap/tap.hh>

#include <boost/algorithm/string/replace.hpp>

using namespace std;

namespace grammar {

  namespace graph {

    namespace dot {

      Printer::Printer(std::ostream &o)
        :
          o_(o)
      {
        auto r = grammar::tap::well_known_tags();
        stop_names_.insert(r.first, r.second);
        stop_names_.insert("INTEGER");
        stop_names_.insert("OCTET STRING");
        stop_names_.insert("OperatorSpecInfoList");
        stop_names_.insert("Code");
      }
      const std::string &Printer::target_namespace() const
      {
        return target_namespace_;
      }
      void Printer::set_target_namespace(const std::string &tn)
      {
        target_namespace_ = tn;
      }
      std::ostream &Printer::o()
      {
        return o_;
      }

      Printer &operator<<(Printer &o, const Grammar &g)
      {
        o.o() << "digraph foo {\n";
        o.o() << "  rankdir=LR;\n";
        o.o() << "  nodesep=\"0.55\"\n";
        for (auto &nt : g.nts()) {
          o << *nt;
          o.o() << "\n\n";
        }
        o.o() << "}\n";
        return o;
      }

      void Printer::print(const Symbol::NT &nt)
      {
        for (auto &ref : nt.rule().refs()) {
          if (stop_names_.count(ref->symbol_name())) {
            o_ << "  t_ref_" << t_ref_counter_ << " [label=\""
              << ref->symbol_name() << "\"];\n";
            o_ << "  \"" << nt.name() << "\" -> t_ref_" << t_ref_counter_;
            ++t_ref_counter_;
          } else {
            o_ << "  \"" << nt.name() << "\" -> \"" << ref->symbol_name()
              << '"';
          }
          if (ref->optional())
            o_ << "[style=\"dashed\"]";
          o_ << ";\n";
        }
        switch (nt.rule().type()) {
          case Rule::SEQUENCE:
          case Rule::ALL:
            o_ << "  " << nt.name() << " [shape=\"rect\"];\n";
            break;
          case Rule::LIST:
          case Rule::SET:
            o_ << "  " << nt.name() << " [shape=\"house\"];\n";
            break;
          case Rule::CHOICE:
            o_ << "  " << nt.name() << " [shape=\"octagon\"];\n";
            break;
          default:
            break;
        }
      }

      Printer &operator<<(Printer &o, const Symbol::NT &nt)
      {
        o.print(nt);
        return o;
      }
      Printer &operator<<(Printer &o, const Symbol::Terminal &terminal)
      {
        return o;
      }
      Printer &operator<<(Printer &o, const Rule::Base &rule)
      {
        rule.accept(o);
        return o;
      }
      Printer &operator<<(Printer &o, const Rule::Link &rule)
      {
        return o;
      }
      Printer &operator<<(Printer &o, const Rule::Sequence &rule)
      {
        return o;
      }
      Printer &operator<<(Printer &o, const Rule::List &rule)
      {
        return o;
      }
      Printer &operator<<(Printer &o, const Rule::Set &rule)
      {
        return o;
      }
      Printer &operator<<(Printer &o, const Rule::Choice &rule)
      {
        return o;
      }
      Printer &operator<<(Printer &o, const Rule::All &rule)
      {
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

    }

  }

}
