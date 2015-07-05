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
#ifndef GRAMMAR_XML_XSD_HH
#define GRAMMAR_XML_XSD_HH

#include "printer.hh"

#include <unordered_map>
#include <string>

namespace grammar {

  namespace xml {

    namespace xsd {

      class Printer : public grammar::xml::Printer {
        private:
          std::unordered_map<std::string, std::string>
            nt_key_defs_;

        public:
          using grammar::xml::Printer::Printer;

          void visit(const Grammar &r) override;
          void visit(const Symbol::NT &r) override;
          void visit(const Symbol::Terminal &r) override;
          void visit(const Rule::Link &r) override;
          void visit(const Rule::Choice &r) override;
          void visit(const Rule::Sequence &r) override;
          void visit(const Rule::All &r) override;
          void visit(const Rule::Set &r) override;
          void visit(const Rule::List &r) override;
          void visit(const Constraint::Size &r) override;
          void visit(const Constraint::Domain &r) override;
          void visit(const Constraint::Enum &r) override;
          void visit(const Constraint::Pattern &r) override;

          void read_key_def(const std::string &nt, const std::string &filename);

          void print_key_defs(const std::string &nt);
      };



    }

  }

}


#endif
