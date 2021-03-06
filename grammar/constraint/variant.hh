/**************************************************************************/
/*
   Autogenerated by the Variant Generator.

   Call was:

     '../variant-generator/build/mkvariant' 'constraint_variant.inf' '--output' 'grammar/constraint/variant.cc'
                                                                          */
/**************************************************************************/
#ifndef CONSTRAINT_VARIANT_HH
#define CONSTRAINT_VARIANT_HH

#include <stdexcept>

#include <grammar/grammar.hh>

namespace grammar {
namespace Constraint {

class Variant {
  private:
    uint8_t tag_ {0};
    union {
      Enum enum_;
      Domain domain_;
      Size size_;
      Pattern pattern_;
    };
    void destruct();
  public:
    Variant();
    Variant(Variant &&o);
    Variant(const Variant &o) =delete;
    Variant(Enum &&o);
    Variant(Domain &&o);
    Variant(Size &&o);
    Variant(Pattern &&o);
    ~Variant();

    Variant &operator=(Variant &&o);
    Variant &operator=(const Variant &o) =delete;
    Variant &operator=(Enum &&o);
    Variant &operator=(Domain &&o);
    Variant &operator=(Size &&o);
    Variant &operator=(Pattern &&o);
    template <typename T> typename T::result_type apply(T &t)
    {
      switch (tag_) {
        case 1 : return t(enum_);
        case 2 : return t(domain_);
        case 3 : return t(size_);
        case 4 : return t(pattern_);
      }
      throw std::domain_error("variant not initialized");
    }

    template <typename T> typename T::result_type apply(const T &t)
    {
      switch (tag_) {
        case 1 : return t(enum_);
        case 2 : return t(domain_);
        case 3 : return t(size_);
        case 4 : return t(pattern_);
      }
      throw std::domain_error("variant not initialized");
    }

    template <typename T> typename T::result_type accept(T &t) const
    {
      switch (tag_) {
        case 1 : return t(enum_);
        case 2 : return t(domain_);
        case 3 : return t(size_);
        case 4 : return t(pattern_);
      }
      throw std::domain_error("variant not initialized");
    }

    template <typename T> typename T::result_type accept(const T &t) const
    {
      switch (tag_) {
        case 1 : return t(enum_);
        case 2 : return t(domain_);
        case 3 : return t(size_);
        case 4 : return t(pattern_);
      }
      throw std::domain_error("variant not initialized");
    }

};

} // grammar
} // Constraint



#endif
