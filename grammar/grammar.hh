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
#ifndef GRAMMAR_GRAMMAR_HH
#define GRAMMAR_GRAMMAR_HH

#include <stdint.h>
#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <stdexcept>

namespace grammar {

  class Runtime_Error : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;
  };

  class Parse_Error : public Runtime_Error {
    public:
      using Runtime_Error::Runtime_Error;
  };

  class Visitor;


  namespace Constraint {

    enum class Source { ASN1, EXTERN };

    class Base {
      private:
        std::string description_;
        Source source_ {Source::ASN1};
      public:
        virtual ~Base();
        virtual void accept(Visitor &v) const = 0;

        const std::string &description() const;
        void set_description(const std::string &d);
        void set_description(std::string &&d);
        void set_source(Source s);
        Source source() const;
    };
    // closed
    class Range_Base : public Base {
      protected:
        std::pair<size_t, size_t> range_;
      public:
        Range_Base();
        Range_Base(size_t first);
        Range_Base(std::pair<size_t, size_t> &&range);

        const std::pair<size_t, size_t> &range() const;
        void set_min(size_t v);
        void set_max(size_t v);
    };
    class Size : public Range_Base {
      private:
      public:
        using Range_Base::Range_Base;
        void accept(Visitor &v) const override;
    };
    // closed
    class Domain : public Base {
      private:
        ssize_t min_{0};
        ssize_t max_{0};
        bool has_min_ {false};
        bool has_max_ {false};
      public:
        Domain();
        Domain(ssize_t val, bool min);
        Domain(ssize_t min_val, ssize_t max_val);
        bool has_min() const;
        bool has_max() const;
        ssize_t min() const;
        ssize_t max() const;
        void set_min(ssize_t v);
        void set_max(ssize_t v);
        void accept(Visitor &v) const override;
    };
    class Pattern : public Base {
      private:
        std::string s_;
      public:
        Pattern(const std::string &s);
        void accept(Visitor &v) const override;

        const std::string &str() const;
    };
    class Enum : public Base {
      private:
        std::deque<std::string> v_;
      public:
        Enum();
        void accept(Visitor &v) const override;

        void push(const std::string &s);
        const std::deque<std::string> &values() const;
    };
  }

  class Coordinates {
    public:
      enum class Type {
        NOT_SPECIFIED,
        IMPLICIT,
        EXPLICIT
      };
    private:
      uint32_t tag_ {0};
      uint32_t klasse_ {0};
      bool initialized_;
      Type type_ {Type::NOT_SPECIFIED};

    public:
      Coordinates();
      Coordinates(uint32_t tag, int klasse);

      bool initialized() const;
      void set(uint32_t tag, int klasse);
      void set_tag(uint32_t tag);
      void set_klasse(uint32_t klasse);
      uint32_t tag() const;
      uint32_t klasse() const;
  };

  namespace Symbol { class Base; class NT; }
  namespace Rule { class Base; }

  class Reference {
    private:
      Coordinates coord_;
      std::string name_;
      std::string symbol_name_;
      Symbol::Base *symbol_ {nullptr};
      bool optional_ {false};
      std::deque<std::unique_ptr<Constraint::Base> > constraints_;
      Rule::Base *parent_  {nullptr};
    public:
      Reference();
      Reference(std::string &&symbol_name);
      void set_name(std::string &&name);
      void set_symbol_name(std::string &&name);
      const std::string &name() const;
      const std::string &symbol_name() const;
      void set_optional(bool b);
      bool optional() const;
      void push(std::unique_ptr<Constraint::Base> constraint);
      const std::deque<std::unique_ptr<Constraint::Base> > &constraints() const;
      std::deque<std::unique_ptr<Constraint::Base> > &constraints();
      void set_constraints(std::deque<std::unique_ptr<Constraint::Base> > &&o);
      void append_constraints(std::deque<std::unique_ptr<Constraint::Base> > &&o);
      void push_constraint(std::unique_ptr<Constraint::Base> c);
      void set_symbol(Symbol::Base &symbol);
      const Symbol::Base &symbol() const;
      Symbol::Base &symbol();
      const Coordinates &coord() const;
      void set_coord(Coordinates &&coord);
      void set_coord(const Coordinates &coord);
      void set_parent(Rule::Base &parent);
      const Rule::Base &parent() const;
      Rule::Base &parent();
  };

  namespace Rule {
    enum Type : uint8_t {
      CHOICE,
      SEQUENCE,
      ALL,
      LIST,
      SET,
      LINK
    };
    class Link;
    class Set;
    class Base {
      protected:
        std::deque<Reference> refs_;
        bool extensible_ {false};
        Symbol::NT *parent_ {nullptr};
      public:
        virtual ~Base();
        void push(Reference &&ref);
        const std::deque<Reference> &refs() const;
        std::deque<Reference> &refs();
        virtual void accept(Visitor &v) const = 0;
        virtual void apply(Visitor &v) = 0;
        virtual Type type() const = 0;
        bool extensible() const;
        void set_extensible(bool e);
        void set_parent(Symbol::NT &parent);
        const Symbol::NT &parent() const;
        void erase_reference(Reference &r);

    };
    class Choice : public Base {
      private:
      public:
        void accept(Visitor &v) const override;
        void apply(Visitor &v) override;
        Type type() const override;
    };
    class Sequence : public Base {
      private:
      public:
        void accept(Visitor &v) const override;
        void apply(Visitor &v) override;
        Type type() const override;
    };
    // ASN.1: Set
    class All : public Base {
      private:
      public:
        void accept(Visitor &v) const override;
        void apply(Visitor &v) override;
        Type type() const override;
    };
    class List : public Base {
      private:
      public:
        List();
        List(std::string &&name);
        void accept(Visitor &v) const override;
        void apply(Visitor &v) override;
        Type type() const override;
    };
    class Set : public Base {
      private:
      public:
        Set();
        Set(std::string &&name);
        void accept(Visitor &v) const override;
        void apply(Visitor &v) override;
        Type type() const override;
    };
    class Link : public Base {
      private:
      public:
        Link();
        Link(std::string &&name);
        void accept(Visitor &v) const override;
        void apply(Visitor &v) override;
        Type type() const override;
    };
  }

  namespace Symbol {
    class Base {
      protected:
        std::string name_;
        Coordinates coord_;
        std::deque<Reference*> parents_;
      public:
        Base();
        Base(const std::string &name);
        Base(std::string &&name);
        Base(std::string &&name, Coordinates &&coord);
        virtual ~Base();
        const std::string &name() const;
        void set_coord(Coordinates &&coord);
        void set_coord(const Coordinates &coord);
        const Coordinates &coord() const;
        void push_parent(Reference &ref);
        const std::deque<Reference*> &parents() const;
        void erase_parent(Reference &ref);
    };
    class Terminal : public Base {
      private:
      public:
        using Base::Base;
    };

    class NT : public Base {
      private:
        std::unique_ptr<Rule::Base> rule_;
      public:
        using Base::Base;
        void set_rule(std::unique_ptr<Rule::Base> rule);
        const Rule::Base &rule() const;
        Rule::Base &rule();
    };
  }

  class Grammar {
    protected:
      std::deque<Symbol::Base*> symbols_; // owns them
      std::deque<Symbol::NT*> nts_;
      Symbol::NT *axiom_ {nullptr};
      std::deque<Symbol::Terminal*> terminals_;
      std::unordered_map<std::string, Symbol::Base*>  name_to_symbol_;
      std::unordered_map<std::string, Symbol::NT*>  name_to_nt_;
      std::unordered_map<std::string, Symbol::Terminal*>  name_to_terminal_;
      bool ignore_duplicates_ {false};
    public:
      Grammar();
      Grammar(Grammar &&g);
      Grammar(const Grammar &) =delete;
      ~Grammar();
      Grammar &operator=(Grammar &&g);
      Grammar &operator=(const Grammar &) =delete;
      void push(std::unique_ptr<Symbol::NT> p);
      void push(std::unique_ptr<Symbol::Terminal> p);
      void erase(const std::string &name);
      const std::deque<Symbol::NT*> &nts() const;
      const std::deque<Symbol::Terminal*> &terminals() const;
      const std::deque<Symbol::Base*> &symbols() const;
      const Symbol::Base &name_to_symbol(const std::string &name) const;
      Symbol::Base &name_to_symbol(const std::string &name);
      const Symbol::NT &name_to_nt(const std::string &name) const;
      Symbol::NT &name_to_nt(const std::string &name);
      bool contains_symbol(const std::string &name) const;
      const Symbol::NT &axiom() const;
      void set_axiom(const std::string &name);
      void set_nts(std::deque<Symbol::NT*> &&nts);
      void set_ignore_duplicates(bool b);

      void init_links();
  };


  class Visitor {
    public:
      virtual void visit(Grammar &r);
      virtual void visit(Symbol::NT &r);
      virtual void visit(Symbol::Terminal &r);
      virtual void visit(const Grammar &r);
      virtual void visit(const Symbol::NT &r);
      virtual void visit(const Symbol::Terminal &r);
      virtual void visit(Rule::Choice &r);
      virtual void visit(Rule::Sequence &r);
      virtual void visit(Rule::All &r);
      virtual void visit(Rule::List &r);
      virtual void visit(Rule::Set &r);
      virtual void visit(Rule::Link &r);
      virtual void visit(const Rule::Choice &r);
      virtual void visit(const Rule::Sequence &r);
      virtual void visit(const Rule::All &r);
      virtual void visit(const Rule::List &r);
      virtual void visit(const Rule::Set &r);
      virtual void visit(const Rule::Link &r);
      virtual void visit(Constraint::Size &r);
      virtual void visit(Constraint::Domain &r);
      virtual void visit(Constraint::Pattern &r);
      virtual void visit(Constraint::Enum &r);
      virtual void visit(const Constraint::Size &r);
      virtual void visit(const Constraint::Domain &r);
      virtual void visit(const Constraint::Pattern &r);
      virtual void visit(const Constraint::Enum &r);
  };

  void derive_tags(Grammar &g);

  void print_tag_translations(std::ostream &o, const Grammar &g, uint32_t klasse);

  std::unordered_map<uint32_t, std::string> tag_translations(
      const Grammar &g, uint32_t klasse);

  std::unordered_set<uint32_t> tag_closure(const Grammar &g,
      const std::string &name, uint32_t klasse);

  void print_tag_closure(std::ostream &o, const Grammar &g,
      const std::string &name, uint32_t klasse);

  const Symbol::Base &effective_symbol(const Reference &ref);

  std::unordered_set<std::string> unreachable_symbols(const Grammar &g);
  void print_unreachable_symbols(std::ostream &o, const Grammar &g);
  void erase_unreachable_symbols(Grammar &g);

  std::unordered_set<uint32_t> primitive_tags(const Grammar &g, uint32_t klasse);
  void print_primitive_tags(std::ostream &o, const Grammar &g, uint32_t klasse);

  std::unordered_map<std::string, std::pair<uint32_t, uint32_t> > map_name_to_klasse_tag(
      const Grammar &g);
  std::unordered_map<std::string, std::tuple<bool, uint32_t, uint32_t> >
      map_name_to_shape_klasse_tag(const Grammar &g);
  void print_name_to_shape_klasse_tag_map(std::ostream &o, const Grammar &g);


  void add_constraints(Grammar &g, const std::string &filename);

}

#endif
