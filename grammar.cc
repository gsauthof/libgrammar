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

#include <exception>
#include <map>
#include <set>
#include <algorithm>

// XXX remove again
#include <iostream>

#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>
#include <ixxx/util.hh>

#include <asn1/grammar.hh>

using namespace std;

namespace grammar {

  Coordinates::Coordinates()
    :
      initialized_(false)
  {
  }
  Coordinates::Coordinates(uint32_t tag, int klasse)
    :
      tag_(tag),
      klasse_(klasse),
      initialized_(true)
  {
  }
  bool Coordinates::initialized() const
  {
    return initialized_;
  }
  void Coordinates::set(uint32_t tag, int klasse)
  {
    tag_ = tag;
    klasse_ = klasse;
    initialized_ = true;
  }
  void Coordinates::set_tag(uint32_t tag)
  {
    tag_ = tag;
    initialized_ = true;
  }
  void Coordinates::set_klasse(uint32_t klasse)
  {
    klasse_ = klasse;
    initialized_ = true;
  }
  uint32_t Coordinates::tag() const
  {
    return tag_;
  }
  uint32_t Coordinates::klasse() const
  {
    return klasse_;
  }

  namespace Constraint {

    Base::~Base()
    {
    }

    const std::string &Base::description() const
    {
      return description_;
    }
    void Base::set_description(const std::string &d)
    {
      description_ = d;
    }
    void Base::set_description(std::string &&d)
    {
      description_ = std::move(d);
    }
    void Base::set_source(Source s)
    {
      source_ = s;
    }
    Source Base::source() const
    {
      return source_;
    }

    Range_Base::Range_Base() = default;
    Range_Base::Range_Base(size_t first)
      :
        range_(first, first)
    {
    }
    Range_Base::Range_Base(std::pair<size_t, size_t> &&range)
      :
        range_(std::move(range))
    {
    }
    const std::pair<size_t, size_t> &Range_Base::range() const
    {
      return range_;
    }
    void Range_Base::set_min(size_t v)
    {
      range_.first = v;
    }
    void Range_Base::set_max(size_t v)
    {
      range_.second = v;
    }
    void Size::accept(Visitor &v) const
    {
      v.visit(*this);
    }
    Domain::Domain()
      :
        has_min_(false),
        has_max_(false)
    {
    }
    Domain::Domain(ssize_t val, bool min)
    {
      if (min) {
        min_ = val;
        has_min_ = true;
      } else {
        max_ = val;
        has_max_ = true;
      }
    }
    Domain::Domain(ssize_t min_val, ssize_t max_val)
      :
        min_(min_val),
        max_(max_val),
        has_min_(true),
        has_max_(true)
    {
    }
    bool Domain::has_min() const
    {
      return has_min_;
    }
    bool Domain::has_max() const
    {
      return has_max_;
    }
    ssize_t Domain::min() const
    {
      if (!has_min_)
        throw logic_error("Range has no min value");
      return min_;
    }
    ssize_t Domain::max() const
    {
      if (!has_max_)
        throw logic_error("Range has no min value");
      return max_;
    }
    void Domain::set_min(ssize_t v)
    {
      min_ = v;
      has_min_ = true;
    }
    void Domain::set_max(ssize_t v)
    {
      max_ = v;
      has_max_ = true;
    }
    void Domain::accept(Visitor &v) const
    {
      v.visit(*this);
    }

    Pattern::Pattern(const std::string &s)
      :
        s_(s)
    {
    }
    const std::string &Pattern::str() const
    {
      return s_;
    }
    void Pattern::accept(Visitor &v) const
    {
      v.visit(*this);
    }
    Enum::Enum()
    {
    }
    void Enum::accept(Visitor &v) const
    {
      v.visit(*this);
    }
    void Enum::push(const std::string &s)
    {
      v_.push_back(s);
    }
    const std::deque<std::string> &Enum::values() const
    {
      return v_;
    }
  }
  Reference::Reference()
  {
  }
  Reference::Reference(std::string &&symbol_name)
    :
      symbol_name_(std::move(symbol_name))
  {
  }
  void Reference::set_name(std::string &&name)
  {
    name_ = std::move(name);
  }
  void Reference::set_symbol_name(std::string &&name)
  {
    symbol_name_ = std::move(name);
  }
  const std::string &Reference::name() const
  {
    return name_;
  }
  const std::string &Reference::symbol_name() const
  {
    return symbol_name_;
  }
  void Reference::set_optional(bool b)
  {
    optional_ = b;
  }
  bool Reference::optional() const
  {
    return optional_;
  }
  void Reference::push(std::unique_ptr<Constraint::Base> constraint)
  {
    constraints_.push_back(std::move(constraint));
  }
  const std::deque<std::unique_ptr<Constraint::Base> > &
    Reference::constraints() const
  {
    return constraints_;
  }
  std::deque<std::unique_ptr<Constraint::Base> > &Reference::constraints()
  {
    return constraints_;
  }
  void Reference::set_constraints(
      std::deque<std::unique_ptr<Constraint::Base> > &&o)
  {
    constraints_ = std::move(o);
  }
  void Reference::append_constraints(
      std::deque<std::unique_ptr<Constraint::Base> > &&o)
  {
    for (auto &e : o)
      constraints_.push_back(std::move(e));
  }
  void Reference::push_constraint(std::unique_ptr<Constraint::Base> c)
  {
    constraints_.push_back(std::move(c));
  }
  void Reference::set_symbol(Symbol::Base &symbol)
  {
    symbol_ = &symbol;
  }
  const Symbol::Base &Reference::symbol() const
  {
    if (!symbol_)
      throw Runtime_Error("Symbol link not initialized: " + symbol_name_);
    return *symbol_;
  }
  Symbol::Base &Reference::symbol()
  {
    if (!symbol_)
      throw Runtime_Error("Symbol link not initialized" + symbol_name_);
    return *symbol_;
  }
  const Coordinates &Reference::coord() const
  {
    return coord_;
  }
  void Reference::set_coord(Coordinates &&coord)
  {
    coord_ = std::move(coord);
  }
  void Reference::set_coord(const Coordinates &coord)
  {
    coord_ = coord;
  }
  void Reference::set_parent(Rule::Base &parent)
  {
    parent_ = &parent;
  }
  const Rule::Base &Reference::parent() const
  {
    return *parent_;
  }
  Rule::Base &Reference::parent()
  {
    return *parent_;
  }

  namespace Rule {
    Base::~Base()
    {
    }
    void Base::push(Reference &&ref)
    {
      refs_.push_back(std::move(ref));
      refs_.back().set_parent(*this);
    }
    const std::deque<Reference> &Base::refs() const
    {
      return refs_;
    }
    std::deque<Reference> &Base::refs()
    {
      return refs_;
    }
    void Base::erase_reference(Reference &r)
    {
      auto i = refs_.begin();
      for (; i != refs_.end(); ++i)
        if (&(*i) == &r) {
          refs_.erase(i);
          return;
        }
    }
    bool Base::extensible() const
    {
      return extensible_;
    }
    void Base::set_extensible(bool e)
    {
      extensible_ = e;
    }
    void Base::set_parent(Symbol::NT &parent)
    {
      parent_ = &parent;
    }
    const Symbol::NT &Base::parent() const
    {
      return *parent_;
    }

    List::List()
    {
    }
    List::List(std::string &&name)
    {
      refs_.emplace_back(std::move(name));
      refs_.back().set_parent(*this);
    }
    Set::Set()
    {
    }
    Set::Set(std::string &&name)
    {
      refs_.emplace_back(std::move(name));
      refs_.back().set_parent(*this);
    }
    Link::Link()
    {
    }
    Link::Link(std::string &&name)
    {
      refs_.emplace_back(std::move(name));
      refs_.back().set_parent(*this);
    }

    void Choice::accept(Visitor &v) const
    {
      v.visit(*this);
    }
    void Sequence::accept(Visitor &v) const
    {
      v.visit(*this);
    }
    void All::accept(Visitor &v) const
    {
      v.visit(*this);
    }
    void List::accept(Visitor &v) const
    {
      v.visit(*this);
    }
    void Set::accept(Visitor &v) const
    {
      v.visit(*this);
    }
    void Link::accept(Visitor &v) const
    {
      v.visit(*this);
    }

    void Choice::apply(Visitor &v)
    {
      v.visit(*this);
    }
    void Sequence::apply(Visitor &v)
    {
      v.visit(*this);
    }
    void All::apply(Visitor &v)
    {
      v.visit(*this);
    }
    void List::apply(Visitor &v)
    {
      v.visit(*this);
    }
    void Set::apply(Visitor &v)
    {
      v.visit(*this);
    }
    void Link::apply(Visitor &v)
    {
      v.visit(*this);
    }
    Type Choice::type() const
    {
      return CHOICE;
    }
    Type Sequence::type() const
    {
      return SEQUENCE;
    }
    Type All::type() const
    {
      return ALL;
    }
    Type List::type() const
    {
      return LIST;
    }
    Type Set::type() const
    {
      return SET;
    }
    Type Link::type() const
    {
      return LINK;
    }

  }

  namespace Symbol {
    Base::Base()
    {
    }
    Base::Base(const std::string &name)
      :
        name_(name)
    {
    }
    Base::Base(std::string &&name)
      :
        name_(std::move(name))
    {
    }
    Base::Base(std::string &&name, Coordinates &&coord)
      :
        name_(std::move(name)),
        coord_(std::move(coord))
    {
    }
    Base::~Base()
    {
    }
    const std::string &Base::name() const
    {
      return name_;
    }
    void Base::set_coord(Coordinates &&coord)
    {
      coord_ = std::move(coord);
    }
    void Base::set_coord(const Coordinates &coord)
    {
      coord_ = coord;
    }
    const Coordinates &Base::coord() const
    {
      return coord_;
    }
    void Base::push_parent(Reference &ref)
    {
      parents_.push_back(&ref);
    }
    const std::deque<Reference*> &Base::parents() const
    {
      return parents_;
    }
    void Base::erase_parent(Reference &ref)
    {
      auto e = find(parents_.begin(), parents_.end(), &ref);
      if (e != parents_.end())
        parents_.erase(e);

    }

    void NT::set_rule(std::unique_ptr<Rule::Base> rule)
    {
      rule_ = std::move(rule);
      rule_->set_parent(*this);
    }
    const Rule::Base &NT::rule() const
    {
      return *rule_;
    }
    Rule::Base &NT::rule()
    {
      return *rule_;
    }
  }

  Grammar::~Grammar()
  {
    for (auto p : symbols_)
      delete p;
  }
  void Grammar::set_ignore_duplicates(bool b)
  {
    ignore_duplicates_ = b;
  }

  void Grammar::push(std::unique_ptr<Symbol::NT> p)
  {
    if (ignore_duplicates_ && name_to_symbol_.count(p->name()))
      return;
    symbols_.emplace_back();
    auto x = p.release();
    symbols_.back() = x;
    nts_.push_back(x);
    if (!axiom_)
      axiom_ = x;
    if (name_to_symbol_.count(x->name()))
      throw Parse_Error("Symbol already seen: " + x->name());
    name_to_symbol_[x->name()] = x;
    name_to_nt_[x->name()] = x;
  }
  void Grammar::push(std::unique_ptr<Symbol::Terminal> p)
  {
    symbols_.emplace_back();
    auto x = p.release();
    symbols_.back() = x;
    terminals_.push_back(x);
    if (name_to_symbol_.count(x->name()))
      throw Parse_Error("Symbol already seen: " + x->name());
    name_to_symbol_[x->name()] = x;
    name_to_terminal_[x->name()] = x;
  }
  void Grammar::erase(const std::string &name)
  {
    if (axiom_ && axiom_->name() == name)
      axiom_ = nullptr;
    auto i = name_to_symbol_.find(name);
    if (i != name_to_symbol_.end()) {
      auto j = find(symbols_.begin(), symbols_.end(), i->second);
      if (j != symbols_.end())
        symbols_.erase(j);
      Symbol::NT *nt = dynamic_cast<Symbol::NT*>(i->second);
      if (nt) {
        for (auto &ref : nt->rule().refs()) {
          ref.symbol().erase_parent(ref);
        }
        auto k = name_to_nt_.find(name);
        if (k != name_to_nt_.end()) {
          auto l = find(nts_.begin(), nts_.end(), k->second);
          if (l != nts_.end())
            nts_.erase(l);
          name_to_nt_.erase(k);
        }
      } else {
        auto k = name_to_terminal_.find(name);
        if (k != name_to_terminal_.end()) {
          auto l = find(terminals_.begin(), terminals_.end(), k->second);
          if (l != terminals_.end())
            terminals_.erase(l);
          name_to_terminal_.erase(k);
        }
      }
      for (auto &ref : i->second->parents()) {
        ref->parent().erase_reference(*ref);
      }
      delete i->second;
      name_to_symbol_.erase(i);
    }
  }
  const std::deque<Symbol::NT*> &Grammar::nts() const
  {
    return nts_;
  }
  const std::deque<Symbol::Terminal*> &Grammar::terminals() const
  {
    return terminals_;
  }
  const std::deque<Symbol::Base*> &Grammar::symbols() const
  {
    return symbols_;
  }
  Grammar::Grammar()
  {
  }
  Grammar::Grammar(Grammar &&g)
  {
    symbols_ = std::move(g.symbols_);
    g.symbols_.clear();
    nts_ = std::move(g.nts_);
    g.nts_.clear();
    axiom_ = g.axiom_;
    g.axiom_ = nullptr;
    terminals_ = std::move(g.terminals_);
    g.terminals_.clear();
    name_to_symbol_ = std::move(g.name_to_symbol_);
    g.name_to_symbol_.clear();
    name_to_nt_ = std::move(g.name_to_nt_);
    g.name_to_nt_.clear();
    name_to_terminal_ = std::move(g.name_to_terminal_);
    g.name_to_terminal_.clear();
    ignore_duplicates_ = g.ignore_duplicates_;
    g.ignore_duplicates_ = false;
  }
  Grammar &Grammar::operator=(Grammar &&g)
  {
    symbols_ = std::move(g.symbols_);
    g.symbols_.clear();
    nts_ = std::move(g.nts_);
    g.nts_.clear();
    axiom_ = g.axiom_;
    g.axiom_ = nullptr;
    terminals_ = std::move(g.terminals_);
    g.terminals_.clear();
    name_to_symbol_ = std::move(g.name_to_symbol_);
    g.name_to_symbol_.clear();
    name_to_nt_ = std::move(g.name_to_nt_);
    g.name_to_nt_.clear();
    name_to_terminal_ = std::move(g.name_to_terminal_);
    g.name_to_terminal_.clear();
    ignore_duplicates_ = g.ignore_duplicates_;
    g.ignore_duplicates_ = false;
    return *this;
  }
  const Symbol::Base &Grammar::name_to_symbol(const std::string &name) const
  {
    try {
      return *name_to_symbol_.at(name);
    } catch (const exception &e) {
      throw grammar::Runtime_Error("Could not find symbol in grammar: " + name);
    }
  }
  Symbol::Base &Grammar::name_to_symbol(const std::string &name)
  {
    try {
      return *name_to_symbol_.at(name);
    } catch (const exception &e) {
      throw grammar::Runtime_Error("Could not find symbol in grammar: " + name);
    }
  }
  const Symbol::NT &Grammar::name_to_nt(const std::string &name) const
  {
    try {
      return *name_to_nt_.at(name);
    } catch (const exception &e) {
      throw grammar::Runtime_Error("Could not find NT in grammar: " + name);
    }
  }
  Symbol::NT &Grammar::name_to_nt(const std::string &name)
  {
    try {
      return *name_to_nt_.at(name);
    } catch (const exception &e) {
      throw grammar::Runtime_Error("Could not find NT in grammar: " + name);
    }
  }
  bool Grammar::contains_symbol(const std::string &name) const
  {
    return name_to_symbol_.count(name);
  }
  const Symbol::NT &Grammar::axiom() const
  {
    return *axiom_;
  }
  void Grammar::set_axiom(const std::string &name)
  {
    axiom_ = &name_to_nt(name);
  }

  void Grammar::init_links()
  {
    for (auto &nt : nts_)
      for (auto &ref : nt->rule().refs()) {
        auto &symbol = name_to_symbol(ref.symbol_name());
        ref.set_symbol(symbol);
        symbol.push_parent(ref);
      }
  }

  void Grammar::set_nts(std::deque<Symbol::NT*> &&nts)
  {
    nts_ = std::move(nts);
  }

  void Visitor::visit(Grammar &r)
  {
  }
  void Visitor::visit(Symbol::NT &r)
  {
  }
  void Visitor::visit(Symbol::Terminal &r)
  {
  }
  void Visitor::visit(const Grammar &r)
  {
  }
  void Visitor::visit(const Symbol::NT &r)
  {
  }
  void Visitor::visit(const Symbol::Terminal &r)
  {
  }
  void Visitor::visit(Rule::Choice &r)
  {
  }
  void Visitor::visit(Rule::Sequence &r)
  {
  }
  void Visitor::visit(Rule::All &r)
  {
  }
  void Visitor::visit(Rule::List &r)
  {
  }
  void Visitor::visit(Rule::Set &r)
  {
  }
  void Visitor::visit(Rule::Link &r)
  {
  }
  void Visitor::visit(const Rule::Choice &r)
  {
  }
  void Visitor::visit(const Rule::Sequence &r)
  {
  }
  void Visitor::visit(const Rule::All &r)
  {
  }
  void Visitor::visit(const Rule::List &r)
  {
  }
  void Visitor::visit(const Rule::Set &r)
  {
  }
  void Visitor::visit(const Rule::Link &r)
  {
  }
  void Visitor::visit(Constraint::Size &r)
  {
  }
  void Visitor::visit(Constraint::Domain &r)
  {
  }
  void Visitor::visit(Constraint::Pattern &r)
  {
  }
  void Visitor::visit(Constraint::Enum &r)
  {
  }
  void Visitor::visit(const Constraint::Size &r)
  {
  }
  void Visitor::visit(const Constraint::Domain &r)
  {
  }
  void Visitor::visit(const Constraint::Pattern &r)
  {
  }
  void Visitor::visit(const Constraint::Enum &r)
  {
  }

  // assuming the rules for all IMPLICIT tagging
  //
  // it can be more complicated, e.g. when EXPLICIT with IMPLICT is mixed
  // and/or AUTOMATIC tagging is enabled ...
  void derive_tags(Grammar &g)
  {
    bool has_changed = true;
    size_t i = 0;
    while (has_changed) {
      has_changed = false;
      for (auto &ntp : g.nts()) {
        auto &nt = *ntp;
        if (!nt.coord().initialized()) {
          switch (nt.rule().type()) {
            case Rule::SEQUENCE:
            case Rule::LIST:
              nt.set_coord(Coordinates(asn1::SEQUENCE, asn1::UNIVERSAL));
              break;
            case Rule::ALL:
            case Rule::SET:
              nt.set_coord(Coordinates(asn1::SET, asn1::UNIVERSAL));
              break;
            case Rule::LINK:
              if (nt.rule().refs().front().coord().initialized())
                nt.set_coord(nt.rule().refs().front().coord());
              break;
            case Rule::CHOICE:
              break;
          }
          if (nt.coord().initialized())
            has_changed = true;
        }
        for (auto &ref : nt.rule().refs()) {
          if (!ref.coord().initialized()) {
            if (ref.symbol().coord().initialized())
              ref.set_coord(ref.symbol().coord());
            if (ref.coord().initialized())
              has_changed = true;
          }
        }
      }
      ++i;
      if (i > 1000u) {
        throw Runtime_Error("derive_tags: unrealistic number of iterations");
      }
    }
  }

  template <typename F>
  void for_each_tag_translation(const Grammar &g, uint32_t klasse, F f)
  {
    for (auto &ntp : g.nts()) {
      auto &nt = *ntp;
      if (nt.coord().initialized() && nt.coord().klasse() == klasse) {
        f(nt.coord().tag(), nt.name());
      }
      for (auto &ref : nt.rule().refs()) {
         if (ref.coord().initialized() && ref.coord().klasse() == klasse) {
           f(ref.coord().tag(), ref.symbol_name());
         }
      }
    }
    for (auto &t : g.terminals()) {
      auto &terminal = *t;
      if (terminal.coord().initialized() && terminal.coord().klasse() == klasse) {
        f(terminal.coord().tag(), terminal.name());
      }
    }
  }

  void print_tag_translations(std::ostream &o, const Grammar &g, uint32_t klasse)
  {
    map<uint32_t, const char *> r;
    for_each_tag_translation(g, klasse, [&r](uint32_t tag, const string &name) {
        r[tag] = name.c_str(); });
    for (auto &x : r) {
      o << x.first << ',' << x.second << '\n';
    }
  }

  unordered_map<uint32_t, std::string> tag_translations(
      const Grammar &g, uint32_t klasse)
  {
    std::unordered_map<uint32_t, std::string> r;
    r.max_load_factor(0.75);
    r.reserve(g.symbols().size());
    for_each_tag_translation(g, klasse, [&r](uint32_t tag, const string &name) {
        r[tag] = name; });
    return r;
  }

  unordered_set<uint32_t> tag_closure(const Grammar &g, const std::string &name, uint32_t klasse)
  {
    std::unordered_set<uint32_t> r;
    r.max_load_factor(0.75);
    r.reserve(g.symbols().size());

    // it is ok to hash the pointer values
    unordered_set<const char*> seen;
    seen.reserve(g.symbols().size());

    auto &root = g.name_to_symbol(name);
    if (root.coord().initialized() && root.coord().klasse() == klasse)
      r.insert(root.coord().tag());
    deque<Reference*> stack;
    stack.insert(stack.end(), root.parents().begin(), root.parents().end());
    seen.insert(root.name().c_str());
    unsigned i = 0u;
    while (!stack.empty()) {
      ++i;
      const Reference *ref = stack.back();
      stack.pop_back();
      if (ref->coord().initialized() && ref->coord().klasse() == klasse)
        r.insert(ref->coord().tag());
      const Rule::Base &rule = ref->parent();
      if (rule.type() == Rule::Type::LINK) {
        const Symbol::NT &nt = rule.parent();
        if (!seen.count(nt.name().c_str())) {
          stack.insert(stack.end(), nt.parents().begin(), nt.parents().end());
          seen.insert(nt.name().c_str());
        }
      }
      if (i > 1000)
        throw Runtime_Error("tag_closure: unrealistic number_of_iterations");
    }
    return r;
  }

  void print_tag_closure(std::ostream &o, const Grammar &g,
      const std::string &name, uint32_t klasse)
  {
    auto r = tag_closure(g, name, klasse);
    set<uint32_t> s(r.begin(), r.end());
    o << name;
    for (auto t : s)
      o << ',' << t;
    o << '\n';
  }

  const Symbol::Base &effective_symbol(const Reference &ref)
  {
    auto &root_symbol = ref.parent().parent();
    const Symbol::Base *s = &ref.symbol();
    for (;;) {
      if (s == &root_symbol)
        return *s;
      //if (s->coord().initialized())
      //  return *s;
      const Symbol::NT *nt = dynamic_cast<const Symbol::NT*>(s);
      if (!nt)
        return *s;
      if (nt->rule().type() != Rule::LINK)
        return *s;
      else
        s = &nt->rule().refs().front().symbol();
    }
    return root_symbol;
  }

  std::unordered_set<std::string> unreachable_symbols(const Grammar &g)
  {
    std::unordered_set<std::string> unreachable;
    unreachable.max_load_factor(0.75);
    unreachable.reserve(g.symbols().size());
    for (auto &s : g.symbols())
      unreachable.insert(s->name());
    deque<const Symbol::Base*> stack;
    stack.push_back(&g.axiom());
    unreachable.erase(g.axiom().name());
    while (!stack.empty()) {
      const Symbol::Base *s = stack.back();
      stack.pop_back();
      const Symbol::NT *nt = dynamic_cast<const Symbol::NT*>(s);
      if (nt) {
        for (auto &ref : nt->rule().refs()) {
          if (unreachable.count(ref.symbol_name())) {
            stack.push_back(&ref.symbol());
            unreachable.erase(ref.symbol_name());
          }
        }
      }
    }
    return unreachable;
  }
  void print_unreachable_symbols(ostream &o, const Grammar &g)
  {
    auto r = unreachable_symbols(g);
    set<string> s(r.begin(), r.end());
    o << "unreachable";
    for (auto & x : s)
      o << ',' << x;
    o << '\n';
  }
  void erase_unreachable_symbols(Grammar &g)
  {
    auto r = unreachable_symbols(g);
    for (auto &name : r)
      g.erase(name);
  }

  std::unordered_set<uint32_t> primitive_tags(const Grammar &g, uint32_t klasse)
  {
    std::unordered_set<uint32_t> r;
    std::unordered_set<const Symbol::NT*> parents;
    for (auto &t : g.terminals()) {
      for (auto &ref : t->parents()) {
        if (ref->parent().type() != Rule::LINK)
          continue;
        const Symbol::NT *nt = &ref->parent().parent();
        parents.insert(nt);
      }
    }
    std::unordered_set<const Symbol::NT*> all_parents(
        parents.begin(), parents.end());
    for (;;) {
      std::unordered_set<const Symbol::NT*> new_parents;
      for (auto & nt : parents) {
        for (auto &ref : nt->parents()) {
          if (ref->parent().type() != Rule::LINK)
            continue;
          const Symbol::NT *nt = &ref->parent().parent();
          if (!all_parents.count(nt) && !new_parents.count(nt)) {
            new_parents.insert(nt);
          }
        }
      }
      if (new_parents.empty())
        break;
      all_parents.insert(new_parents.begin(), new_parents.end());
      swap(parents, new_parents);
    }
    for (auto &nt : all_parents)
      if (nt->coord().klasse() == klasse)
        r.insert(nt->coord().tag());
    return r;
  }
  void print_primitive_tags(std::ostream &o, const Grammar &g, uint32_t klasse)
  {
    auto r = primitive_tags(g, klasse);
    set<uint32_t> s(r.begin(), r.end());
    o << "primitve tags";
    for (auto &e : s)
      o << ',' << e;
    o << '\n';
  }

  void add_constraints(Grammar &g, const std::string &filename)
  {
    unordered_map<string, Constraint::Enum> enums_;
    unordered_map<string, Constraint::Domain> domains_;
    unordered_map<string, Constraint::Size> sizes_;
    ixxx::util::Mapped_File f(filename);
    using si = boost::algorithm::split_iterator<const char*>;
    pair<const char*, const char*> inp(f.s_begin(), f.s_end());
    const char q_s[1] = {0};
    pair<const char*, const char*> q(q_s, q_s + 1);
    for (si i = boost::algorithm::make_split_iterator(inp,
                  boost::algorithm::token_finder(
                     boost::algorithm::is_any_of("\r\n"),
                     boost::algorithm::token_compress_on) );
        i != si(); ++i) {
      si j = boost::algorithm::make_split_iterator(*i,
            boost::algorithm::first_finder(
              q, boost::algorithm::is_equal()));
      if (j != si()) {
        if (boost::empty(*j))
          continue;
        string nt_name = boost::copy_range<string>(*j);
        Symbol::NT &nt = g.name_to_nt(nt_name);
        ++j;
        if (j != si()) {
          string type = boost::copy_range<string>(*j);
          ++j;
          if (j != si()) {
            string value = boost::copy_range<string>(*j);
            if (type == "pattern") {
              nt.rule().refs().front().push_constraint(
                  make_unique<Constraint::Pattern>(value));
            } else if (type == "minInclusive") {
              domains_[nt_name].set_min(boost::lexical_cast<ssize_t>(value));
            } else if (type == "maxInclusive") {
              domains_[nt_name].set_max(boost::lexical_cast<ssize_t>(value));
            } else if (type == "minLength") {
              sizes_[nt_name].set_min(boost::lexical_cast<size_t>(value));
            } else if (type == "maxLength") {
              sizes_[nt_name].set_max(boost::lexical_cast<size_t>(value));
            } else if (type == "enumeration") {
              enums_[nt_name].push(value);
            } else {
              throw runtime_error("Unkwnown constraint type: " + type);
            }
            ++j;
          }
        }
      }
    }
    for (auto &e : enums_)
      g.name_to_nt(e.first).rule().refs().front().push_constraint(
          make_unique<Constraint::Enum>(std::move(e.second)));
    for (auto &e : domains_)
      g.name_to_nt(e.first).rule().refs().front().push_constraint(
          make_unique<Constraint::Domain>(std::move(e.second)));
    for (auto &e : sizes_) {
      e.second.set_source(Constraint::Source::EXTERN);
      g.name_to_nt(e.first).rule().refs().front().push_constraint(
          make_unique<Constraint::Size>(std::move(e.second)));
    }
  }

}
