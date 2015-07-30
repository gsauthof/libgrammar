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
#ifndef GRAMMAR_XML_HH
#define GRAMMAR_XML_HH

#include <ostream>
#include <string>

#include <grammar/grammar.hh>

namespace grammar {

  namespace xml {

      class Printer : public grammar::Visitor {
        private:
          std::string comment_;
        protected:
          std::ostream &o_;
          std::string target_namespace_;

          void print_comment();
        public:
          std::string nt_name_;
          std::string symbol_name_;

          Printer(std::ostream &o);
          std::ostream &o();
          void set_target_namespace(const std::string &tn);
          const std::string &target_namespace() const;
          void set_comment(int argc, char **argv);
          void set_comment(const std::string &s);

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
      Printer &operator<<(Printer &o, const Constraint::Base &c);
      Printer &operator<<(Printer &o, const Constraint::Size &c);

  }

}

#endif
