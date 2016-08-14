SAXON = java -jar /usr/share/java/saxon.jar
DSLFILE = /usr/share/sgml/docbook/stylesheet/dsssl/modular/print/docbook.dsl
XMLDCL = /usr/share/sgml/declaration/xml1n.dcl

%.html: templ/docbook.html.xsl %.xml
	xsltproc -o $@ $^ 
	konwert koi8r-cp1251/html $@ -O

%.fo: %.xml templ/docbook.fo.xsl
	$(SAXON) -o $@ $^

%.latex: templ/docbook.latex.xsl %.xml
	xsltproc -o $@ $^

%.tex: %.xml
	jade -b iso-8859-5 -d templ/docbook.dsssl -t tex -o $@ $(XMLDCL) $<

%.pdf: %.latex
	pdflatex -output-format pdf $<
	rm $(addprefix $(basename $@),.aux .glo .idx .log .out)

%-tex.pdf: %.tex
	pdfjadetex -output-format pdf $<

%-fop.pdf: %.fo
	fop -fo $< -pdf $@

all: midenkov-cv.html midenkov-cv.pdf

.PHONY: all
