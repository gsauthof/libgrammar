C++ Library for common [grammar][4] related tasks.


Currently, it includes a mini-ASN.1 parser and some functions that
are of interest in the context of processing [BER encoded][1] files where
an [ASN.1 specificaton][2] is available. Mini-ASN.1 means that certain
ASN.1 features (e.g. unlimited nesting) are not supported. Thus, the
mini-ASN.1 parser is actually implemented via a regular grammar.
Another (current) restriction is that the algorithms assume that
the ASN.1 grammar uses the all-implicit tagging style. The data structures are
prepared for more complicated tagging styles, though. The mini-ASN.1
is certainly enough for parsing some [GSMA][5] related ASN.1 specifications
(e.g. [TAP][8]/[RAP][9]/NRTRDE - a.k.a. TD.57, TD.32 and TD.35).

Selected functions are:

- create the mapping: `tag -> symbol name` - can be used e.g. for
  displaying a tag name in a BER viewer
- derive tag closure - a tag closure contains all tag aliases of a certain
  symbol, needed for correctly interpreting a BER file
- convert an ASN.1 grammar into a [XML Schema (XSD)][6]
  or [RelaxNG Schema (RNG)][7] - the resulting grammar can be used e.g.
  to validate a BER-to-XML-converted file
- topological sort a grammar - e.g. for cleaning up an existing grammar
- remove unreachable symbols
- concatenate grammar snippets

The unittests and the help screen of the `ged` utility show in detail
how the library features can be used.

2015, Georg Sauthoff <mail@georg.so>


## Unittests

compile via:

    $ mkdir build
    $ cmake ..
    $ make ut

run:

    $ ./ut

## Notes

Some GSMA grammars contain some elements commented out by default, resulting
in a very permissive default grammar. With a sed one-liner such
elements can be commented in, e.g.:

    $ sed -e 's/-- *\((\?SIZE\)/\1/' \
          -e 's/OPTIONAL *\(,\?\) *-- *\*m\.m\./\1/' TAP0312.asn

For checking the validity of generated XSD/RNG files:

    $ xmllint -noout -schema xml/XMLSchema-1.0.xsd foo.xsd
    $ xmllint -noout -relaxng xml/relaxng.rng foo.rng

Note that XSD is not absolutely self describing, i.e. a validating XSD might
still be non-conforming.

A (as of 2015) current version of the TAP specification can be directly
downloaded from the GSMA (no registration necessary). The document describes
version 3.12 (i.e. TAP specification version 3, TAP release version 12), and
the version of the document is 32.3 (released 2014-09-15). Unfortunately, the
GSMA doesn't provide a - say - version controlled repository (e.g. a Git one)
where ASN.1 files can be easiliy downloaded. The GSMA also doesn't provide
direct http links to ASN.1 files or to archive files containing them, either.
Instead, they are inlined in MS Word or PDF files such that one has to copy and
paste them from there ... Following one-liner automates the extraction for PDF
documents:

    $ pdftotext -layout TD.57-v32.31.pdf - \
      | sed -e 's/[^- ]\((SIZE\)/--\1/' -e 's/^represent/--represent/' \
      | awk '/Page.*of/ {next} /^ *Official Document/ {next}
             /^[^-]*GSM Association/ {next}
             /--.*following ASN\.1 specification/ {p=1} p {print}
             /^END/ {p=0}' > TD.57-v32.31.asn1

(This also works for the referenced RAP PDF.)

## References

This repository also contains a few files from the below locations:

- http://relaxng.org/relaxng.rng
- http://www.w3.org/2001/XMLSchema.xsd
- http://www.w3.org/2009/XMLSchema/XMLSchema.xsd
- http://hackage.haskell.org/package/language-asn1-0.5

## License

[LGPLv3+][3]

[1]: https://en.wikipedia.org/wiki/X.690#BER_encoding
[2]: https://en.wikipedia.org/wiki/Abstract_Syntax_Notation_One
[3]: https://www.gnu.org/licenses/lgpl-3.0.en.html
[4]: https://en.wikipedia.org/wiki/Formal_grammar
[5]: https://en.wikipedia.org/wiki/GSM_Association
[6]: https://en.wikipedia.org/wiki/XML_schema
[7]: https://en.wikipedia.org/wiki/RELAX_NG
[8]: http://www.gsma.com/newsroom/wp-content/uploads/TD.57-v32.31.pdf
[9]: http://www.gsma.com/newsroom/wp-content/uploads/TD.32-v6.11.pdf

