// # Parser for a subset of ASN.1.
//
// Less popular features of ASN.1 like for example
// unlimited nesting, value references etc. are not
// implemented.
//
// Still, the subset is enough for a lot of use cases, e.g. to
// parse the ASN.1 grammars included in several GSMA standards
// (TAP, RAP, NRT, ...)

#include <iostream>
#include <string>
#include <utility>
#include <algorithm>
#include <iomanip>
// make_reverse_iterator part of C++14, not available with GCC 4.9
//#include <iterator>
// thus:
#include <boost/iterator/reverse_iterator.hpp>

#include <grammar/grammar.hh>
#include <grammar/asn1/grammar.hh>

#include <boost/lexical_cast.hpp>

#include <ixxx/util.hh>

using namespace std;
using namespace grammar;

%%{

machine asn1_mini;

action save_p {
  last = p;
}

wsc = ( '\n' @{ ++line_; } | [ \t\r\v] ) ;

# Comments are handled outside of this parser, see the Parser class
# (otherwise this parser would get ugly and the number of states would
#  explode)

ws =  wsc + ;

number = [0-9]+
  >save_p %{ }
;

# ## Identifiers
#
# ASN.1 does not allow undescores in identifier, curiosly, it does allow
# dashes, instead ... but an identifier should not contain a double
# dash (as this would conflict single line comments) and it should not
# end with one.
#
# Type, Module etc. identifiers should start with an upper-case letter,
# names with a lower case one.
#
# However, since the underscore does not yield ambiguities or other issues
# it is allowed here - especially since there are legacy grammars in the wild
# that use underscores.
#
# For the same reason, the lexing of lower-case-starting name identifiers
# is relaxed.

#uc_ident = [A-Z] ( '-' | [A-Za-z0-9])* ;
uc_ident = [A-Z] ( '-' | [_A-Za-z0-9])* ;

#lc_ident = [a-z] ( '-' | [A-Za-z0-9])* ;
#lc_ident = [a-z] ( '-' | [_A-Za-z0-9])* ;
lc_ident = [A-Za-z] ( '-' | [_A-Za-z0-9])* ;

module_ident = uc_ident
  >save_p %{ }
;

type_ident = (   ( (uc_ident - ( 'CHOICE' | 'SEQUENCE' | 'SET' | 'OCTET' | 'END' | 'IMPLICIT' | 'EXPLICIT' | 'BIT' | 'OBJECT' | 'IMPORT' | 'SIZE' ) )
 >{ type_ident.first = p;} %{ type_ident.second = p; }
 )
               | ( ('OCTET' ws 'STRING')
                  %{ type_ident = make_pair(octet_string, octet_string+sizeof(octet_string)-1);} )
               | ( ('BIT' ws 'STRING' )
                  %{ type_ident = make_pair(bit_string, bit_string+sizeof(bit_string)-1);} )
               | ( ('OBJECT' ws 'IDENTIFIER')
                  %{ type_ident = make_pair(object_identifier, object_identifier+sizeof(object_identifier)-1);} )
  )
  >save_p %{ }
;

name_ident = ( lc_ident - 'SIZE' )
  >save_p %{ }
;


optional_clause =  'OPTIONAL'
  %{ ref_->set_optional(true); }
;

naked_constraint = 'SIZE' ws? '(' ws? number
  %{
     size_.first = boost::lexical_cast<size_t>(string(last, p));
     size_.second = size_.first;
  }
  (ws? '..' ws? number
  %{
     size_.second = boost::lexical_cast<size_t>(string(last, p));
  }
  )? ws? ')'
  @{
     ref_->push(make_unique<Constraint::Size>(
                 make_pair(size_t(size_.first), size_t(size_.second))));
  }
  ;

# e.g. as in INTEGER (10..100)
number_constraint = number
  %{
     size_.first = boost::lexical_cast<size_t>(string(last, p));
  }
  ws? '..' ws? number
  %{
     size_.second = boost::lexical_cast<size_t>(string(last, p));
     ref_->push(make_unique<Constraint::Domain>(size_.first, size_.second));
  }
  ;

constraint = ws? '(' ws? ( naked_constraint | number_constraint ) ws? ')'
           | ws naked_constraint
  ;


klasse = 'UNIVERSAL' %{ klasse_ = asn1::UNIVERSAL; }
  | 'APPLICATION' %{ klasse_ = asn1::APPLICATION; }
  # ASN.1 does not specify this keyword
  # it is not really needed, since it is the default
  | 'CONTEXT' %{ klasse_ = asn1::CONTEXT_SPECIFIC; }
  | 'PRIVATE' ${ klasse_ = asn1::PRIVATE; }
;

implicit = 'IMPLICIT';
explicit = 'EXPLICIT';

# by default, if no class is specified, the class is context dependent
#
# a tag can be associated to a NT (then directly follows the ::=)
# or to a field of a choice/set/sequence (then it precedes the type)
# (with a named type it comes between the name and the type)
tag = '[' @{ klasse_ = asn1::CONTEXT_SPECIFIC; } (klasse ws)? number
  %{
     coord_ = Coordinates(boost::lexical_cast<size_t>(string(last, p)), klasse_);
  }
']'  ws? ((implicit | explicit) ws )?
;

# i.e. named type reference
named_type = ( name_ident ws
  >{
     ref_->set_name(string(last, p));
  }
  ( tag %{ ref_->set_coord(coord_); } )? type_ident
  %{
     ref_->set_symbol_name(string(type_ident.first, type_ident.second));
  }
   ( (constraint ws? optional_clause?) | (ws optional_clause)?  ) )
  |
  ( '...' %{ rule_->set_extensible(true); } )
  ;

simple_type = type_ident
  %{
      rule_ = make_unique<Rule::Link>(
                string(type_ident.first, type_ident.second));
      ref_ = make_unique<Reference>();
  }
  ( ws ? '(' naked_constraint ws ? ')' 
  %{
     rule_->refs().front()->set_constraints(std::move(ref_->constraints()));
  }
  ) ?
  ;


##sequence = 'SEQUENCE' %{ cerr << "got sequence\n";  }
##  ws '{' ws? named_type ( ws? ',' ws? named_type)* ws? '}'
##;
##
##set = 'SET' %{ cerr << "got set\n"; }
##  ws '{' ws? named_type ( ws? ',' ws? named_type)* ws? '}'
##;
##
##choice = 'CHOICE' %{ cerr << "got choice\n"; }
##  ws '{' ws? named_type ( ws? ',' ws? named_type)* ws? '}'
##;

sequence_set_choice =
  ( 'SEQUENCE' %{ rule_ = make_unique<Rule::Sequence>(); }
  | 'SET' %{ rule_ = make_unique<Rule::All>(); }
  | 'CHOICE' %{ rule_ = make_unique<Rule::Choice>(); } )
  ws? '{'
  %{
     ref_ = make_unique<Reference>();
     coord_ = Coordinates();
  }
  ws? named_type ( ws? ','
  %{
     rule_->push(std::move(ref_));
     ref_ = make_unique<Reference>();
     coord_ = Coordinates();
  }
  ws? named_type)* ws? '}'
  %{
     if (!ref_->name().empty())
       rule_->push(std::move(ref_));
  }
;

sequence_set_of = ( 'SEQUENCE' %{ sequence_ = true; }
  | 'SET' %{ sequence_ = false; }) ws 'OF'
  %{
     if (sequence_)
       rule_ = make_unique<Rule::List>();
     else
       rule_ = make_unique<Rule::Set>();
   }
  ws type_ident %{ rule_->push(make_unique<Reference>(string(type_ident.first, type_ident.second))); } ;


##component_type = sequence | sequence_of | set | choice ;

component_type = sequence_set_of | sequence_set_choice ;

# i.e. type reference
type = simple_type
     | component_type
;



type_assignment = type_ident ws? '::='
  %{
     nt_ = make_unique<Symbol::NT>(string(type_ident.first, type_ident.second));
     coord_ = Coordinates();
  }
  ws? ( tag
  %{
    nt_->set_coord(coord_);
  }
  )? type
;

assignment = type_assignment ;

assignment_list = ws?
  ( assignment ws
  %{
     nt_->set_rule(std::move(rule_));
     grammar_.push(std::move(nt_));
  }
  )+ ;

# the default mode is EXPLICIT
#
# EXPLICIT - a tag in a choice/set/sequence field is additionally encoded
# IMPLICIT - a tag in a choice/set/sequence field replaces the following
# AUTOMATIC - automatically tag the fields in a choice/set/sequence,
#             starting from 0. Those tags are explicit for choice fields
#             (and a few others), but implicit for all others.
#
# the module wide tagging mode can be locally overridden
tagging_mode = ( 'EXPLICIT' | 'IMPLICIT' | 'AUTOMATIC' ) ws 'TAGS' ;

# has the same effect as adding the extension marker '...' to
# each set/sequence/choice/enumerated constructs
extensible_mode = 'EXTENSIBILITY' ws 'IMPLIED'
  ;

module_definition = ws? module_ident ws 'DEFINITIONS' (ws tagging_mode)?
  (ws extensible_mode)?
  ws? '::=' ws?
  'BEGIN' %{ }
  (
    ws 'IMPORTS' ws? uc_ident (ws? ',' ws? uc_ident)*  ws 'FROM'
    ws module_ident ws? ';'
  )?
  ws assignment_list
  ('END' %{ }
  ws )?
  ;

module = module_definition | assignment_list ;

main := module ;

## assignment_list
## assignment = type_assignment
# type_assignment

}%%

#include <grammar/asn1/mini_parser.hh>
#include <stdexcept>

using namespace std;

template< class InputIt, class T >
InputIt find_next( InputIt first, InputIt last, const T& value )
{
  auto r = find(first, last, value);
  if (r < last)
    ++r;
  return r;
}
template< class ForwardIt1, class ForwardIt2 >
ForwardIt1 search_next( ForwardIt1 first, ForwardIt1 last,
                   ForwardIt2 s_first, ForwardIt2 s_last )
{
  auto r = search(first, last, s_first, s_last);
  if (r < last)
    r += s_last - s_first;
  return r;
}

namespace grammar {

  namespace asn1 {

    namespace mini {

      %% write data;

      Parser::Parser(Grammar &&g, ostream &error_stream)
        :
          e(error_stream),
          grammar_(std::move(g))
      {
        %% write init;
      }
      Parser::Parser(ostream &error_stream)
        :
          e(error_stream)
      {
        %% write init;
      }
      void Parser::restart()
      {
        %% write init;
      }
      Parser::Parser(const std::string &filename, std::ostream &error_stream)
        :
          e(error_stream),
          filename_(filename)
      {
        %% write init;
      }
      void Parser::set_filename(const std::string &filename)
      {
        filename_ = filename;
      }

      // asn.1 single line comment is started with a double dash
      // asn.1 dash comment can also be ended by another double dash,
      // whatever comes first ...
      // XXX also multi line comments are allowed with /* */
      // XXX multi line comments must be recursively matched ...
      void Parser::read_without_comments(const char *begin, const char *end)
      {
        eof = nullptr;
        const char ddash[] = "--";
        const char *b = begin;
        do {
          const char *e = search(b, end, ddash, ddash+sizeof(ddash)-1);
          try {
            b = read_section(b, e);
          } catch (const Parse_Error &e) {
            throw_parse_error(begin, p, end, e.what());
          }
          if (e < end) {
            e += sizeof(ddash)-1;
            const char *x = search_next(e, end, ddash, ddash+sizeof(ddash)-1);
            const char *y = find_next(e, end, '\n');
            b = min(x, y);
            size_t newlines = count(e, b, '\n');
            line_ += newlines;
          }
        } while (b < end);
        eof = end;
        read_section(end, end);
      }

      const char *Parser::read_section(const char *begin, const char *end)
      {
        const char octet_string[] = "OCTET STRING";
        const char bit_string[] = "BIT STRING";
        const char object_identifier[] = "OBJECT IDENTIFIER";
        pair<const char*, const char*> type_ident{nullptr, nullptr};
        const char *last = nullptr;
        //const char *p   = begin;
        p = begin;
        const char *pe  = end;
        // is set to pe for the last block
        //const char *eof = nullptr;

        %% write exec;
        if (cs == %%{write error;}%%) {
          throw Parse_Error("parse error");
        }
        return end;
      }
      void Parser::read(const char *begin, const char *end)
      {
        read_without_comments(begin, end);
      }

      void Parser::throw_parse_error(const char *begin,
          const char *p, const char *end, const string &msg)
      {
        auto x = find(boost::make_reverse_iterator(p),
                      boost::make_reverse_iterator(begin),
                      '\n');
        auto y = find(p, end, '\n');
        size_t column = p - x.base() + 1;
        ostringstream o;
        o << filename_ << ":" << line_ << ":" << column
          << ": error: " << msg << "\n"
          << string(x.base(), y) << '\n';
        o << setw(column) << '^';
        //e << o.str() << '\n';
        throw Parse_Error(o.str());
      }

      grammar::Grammar Parser::grammar()
      {
        return std::move(grammar_);
      }

      static Grammar parse_file(const std::string &filename,
          grammar::Grammar &&g)
      {
        auto f = ixxx::util::mmap_file(filename);
        grammar::asn1::mini::Parser parser(std::move(g));
        parser.set_filename(filename);
        parser.read(f.s_begin(), f.s_end());
        return parser.grammar();
      }

      Grammar parse_files(const std::deque<std::string> &filenames)
      {
        auto i = filenames.begin();
        Grammar g;
        g = parse_file(*i, std::move(g));
        g.set_ignore_duplicates(true);
        for (; i != filenames.end(); ++i)
          g = parse_file(*i, std::move(g));
        return g;
      }
    
    }

  }
}
