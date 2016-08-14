# PREFIX = c:/cygwin64
DSLFILE = $(PREFIX)/usr/share/sgml/docbook/stylesheet/dsssl/modular/print/docbook.dsl
XMLDCL = $(PREFIX)/usr/share/sgml/declaration/xml1n.dcl
XSLTHL_JAR = $(PREFIX)/usr/local/lib/xslthl/xslthl.jar
XSLTHL_CONFIG = $(PREFIX)/usr/share/sgml/docbook/xsl-stylesheets/highlighting/xslthl-config.xml
SAXON = java -cp "$(PREFIX)/usr/local/lib/saxon/saxon.jar;$(XSLTHL_JAR)" -Dxslthl.config="file:///$(XSLTHL_CONFIG)" com.icl.saxon.StyleSheet

%.html: templ/docbook.html.xsl %.xml
	xsltproc -o $@ $^

%.fo: %.xml templ/docbook.fo.xsl
	$(SAXON) -o $@ $^

%.latex: templ/docbook.latex.xsl %.xml
	xsltproc -o $@ $^

%.tex: %.xml
	jade -b iso-8859-5 -d templ/docbook.dsssl -t tex -o $@ $(XMLDCL) $<

%-latex.pdf: %.latex
	pdflatex -output-format pdf $<
	rm $(addprefix $(basename $@),.aux .glo .idx .log .out)

%-tex.pdf: %.tex
	pdfjadetex -output-format pdf $<

%-fop.pdf: %.fo
	fop -fo $< -pdf $@

%.tex: %.xml templ/dblatex.xsl
	dblatex $(DBLATEX_OPTS) \
		-b xetex -o $@ \
		-p templ/dblatex.xsl \
		-t tex $<
	./fix-tex.sh $@


%.pdf: %.xml templ/dblatex.xsl
	dblatex $(DBLATEX_OPTS) \
		-r fix-tex.sh \
		-b xetex -o $@ \
		-p templ/dblatex.xsl \
		--pdf $<

html: midenkov-cv-rus.html
pdf: midenkov-cv-rus.pdf
tex: midenkov-cv-rus.tex

install: html pdf
	@if [ -z "$(DEST)" ]; \
	then \
		echo DEST_DIR must be defined! >&2; \
		exit 1; \
	fi
	@cp -at "$(DEST)" files images *.html *.css *.pdf *.xml
	@mv "$(DEST)/midenkov-cv-rus.html" "$(DEST)/index.html"

all: html pdf

.PHONY: all html pdf tex install
