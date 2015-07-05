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
#ifndef GRAMMAR_ASN1_HH
#define GRAMMAR_ASN1_HH

#include <ostream>

#include <grammar.hh>

namespace grammar {

  namespace asn1 {

    enum Type : uint8_t {
      // and SEQUENCE OF
      SEQUENCE                      = 16u,
      // and SET OF
      SET                           = 17u
    };
    enum Klasse : uint8_t {
      UNIVERSAL        = 0u,
      APPLICATION      = 1u,
      // the default class when no class is specified
      // must only be unique inside a SEQUENCE/CHOICE/SET scope
      CONTEXT_SPECIFIC = 2u,
      PRIVATE          = 3u
    };

    void add_terminals(Grammar &g);


    class Printer : public grammar::Visitor {
      private:
        std::ostream &o_;
      public:
        Printer(std::ostream &o);
        std::ostream &o();

        void visit(const Rule::Link &r) override;
        void visit(const Rule::Choice &r) override;
        void visit(const Rule::Sequence &r) override;
        void visit(const Rule::All &r) override;
        void visit(const Rule::Set &r) override;
        void visit(const Rule::List &r) override;
        void visit(const Constraint::Size &r) override;
        void visit(const Constraint::Domain &r) override;
    };

    Printer &operator<<(Printer &o, const Grammar &g);
    Printer &operator<<(Printer &o, const Symbol::Base &symbol);
    Printer &operator<<(Printer &o, const Symbol::NT &nt);
    Printer &operator<<(Printer &o, const Rule::Base &rule);
    Printer &operator<<(Printer &o, const Rule::Link &rule);
    Printer &operator<<(Printer &o, const Rule::Choice &rule);
    Printer &operator<<(Printer &o, const Rule::Sequence &rule);
    Printer &operator<<(Printer &o, const Rule::All &rule);
    Printer &operator<<(Printer &o, const Rule::Set &rule);
    Printer &operator<<(Printer &o, const Rule::List &rule);
    Printer &operator<<(Printer &o, const Reference &ref);
    Printer &operator<<(Printer &o, const Constraint::Base &c);
    Printer &operator<<(Printer &o, const Constraint::Size &c);
    Printer &operator<<(Printer &o, const Constraint::Domain &c);
    Printer &operator<<(Printer &o, const Coordinates &coord);

  }

}

#endif
