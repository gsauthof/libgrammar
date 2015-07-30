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
#ifndef GRAMMAR_GRAPH_DOT_HH
#define GRAMMAR_GRAPH_DOT_HH

#include <ostream>
#include <string>
#include <set>

#include <grammar/grammar.hh>

namespace grammar {

  namespace graph {

    namespace dot {

      class Printer : public grammar::Visitor {
        private:
          std::ostream &o_;
          std::string target_namespace_;
          unsigned t_ref_counter_{0};
          std::set<std::string> stop_names_;
        public:
          Printer(std::ostream &o);
          std::ostream &o();
          void set_target_namespace(const std::string &tn);
          const std::string &target_namespace() const;

          void print(const Symbol::NT &nt);

          void visit(const Rule::Link &r) override;
          void visit(const Rule::Choice &r) override;
          void visit(const Rule::Sequence &r) override;
          void visit(const Rule::All &r) override;
          void visit(const Rule::Set &r) override;
          void visit(const Rule::List &r) override;
      };

      Printer &operator<<(Printer &o, const Grammar &g);
      Printer &operator<<(Printer &o, const Symbol::NT &nt);
      Printer &operator<<(Printer &o, const Symbol::Terminal &terminal);
      Printer &operator<<(Printer &o, const Rule::Base &rule);
      Printer &operator<<(Printer &o, const Rule::Link &rule);
      Printer &operator<<(Printer &o, const Rule::Choice &rule);
      Printer &operator<<(Printer &o, const Rule::Sequence &rule);
      Printer &operator<<(Printer &o, const Rule::All &rule);
      Printer &operator<<(Printer &o, const Rule::Set &rule);
      Printer &operator<<(Printer &o, const Rule::List &rule);

    }

  }

}


#endif
