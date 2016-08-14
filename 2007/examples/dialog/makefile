GFF2XML = gfftoxml
XML2GFF = xmltogff
DLGDIR = c:/games/nwn/override
STYLESHEET = dialog.xsl

XSLT = java net.sf.saxon.Transform
# XSLT = c:/progra~1/sablot/sabcmd

ifeq ($(notdir $(XSLT)), sabcmd)
# Sablotron
XSL_PARSE = $(XSLT) $(STYLESHEET) $< $@

else
# Saxon
export CLASSPATH = C:\Progra~1\Saxon\saxon8sa.jar
XSL_PARSE = $(XSLT) $< $(STYLESHEET) > $@
endif

INDIR = in
OUTDIR = out

infiles := $(basename $(notdir $(wildcard $(DLGDIR)/*.dlg)))
xmlfiles := $(addsuffix .xml, $(infiles))
outfiles := $(basename $(wildcard *.xml))
dlgfiles := $(addsuffix .dlg, $(outfiles))

dlg: $(dlgfiles)

xml: $(xmlfiles)

install: dlg
	cp -u $(OUTDIR)/*.dlg $(DLGDIR)
	
$(outfiles): %: $(OUTDIR)/%.dlg.TIMESTAMP

$(dlgfiles): %: $(OUTDIR)/%.TIMESTAMP

$(infiles): %: $(INDIR)/%.xml

$(xmlfiles): %: $(INDIR)/%

$(INDIR)/%.dlg.xml: $(DLGDIR)/%.dlg
	$(GFF2XML) $(INDIR) $<

$(INDIR)/%.xml: $(INDIR)/%.dlg.xml
	$(XSL_PARSE)
	
$(OUTDIR)/%.p2.xml: %.xml
	$(XSL_PARSE)
	
$(OUTDIR)/%.dl2.xml: $(OUTDIR)/%.p2.xml
	$(XSL_PARSE)
	
$(OUTDIR)/%.dlg.TIMESTAMP: $(OUTDIR)/%.dl2.xml
	$(XML2GFF) $(OUTDIR) $<
	touch $@
	rm -f System.out

.PHONY: xml dlg install $(infiles) $(outfiles) $(xmlfiles) $(dlgfiles)
