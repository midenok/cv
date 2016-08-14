<?xml version="1.0" encoding="UTF8"?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
  <xsl:param name="l10n.gentext.language" select="'ru'"/>
  <xsl:param name="section.autolabel" select="1"/>
  <xsl:param name="latex.encoding" select="'utf8'"/>
  <xsl:param name="latex.babel.language" select="'russian'"/>
  <xsl:param name="doc.collab.show" select="0"/>
  <xsl:param name="latex.output.revhistory" select="1"/>
  <xsl:param name="latex.documentclass.book" select="'a4paper,12pt,notitlepage,oneside,openany'"/>
  <xsl:param name="latex.hyperparam" select="'colorlinks,linkcolor=blue'"/>
  <xsl:param name="doc.section.depth" select="5"/>
  <xsl:param name="hyphenation.format" select="'nohyphen'"/>
  <xsl:param name="doc.lot.show" select="''"/>
  <xsl:param name="doc.layout">coverpage toc frontmatter mainmatter index </xsl:param>

  <!-- dblatex cant understand graphics with ulinks -->
  <xsl:template match="ulink[inlinemediaobject|mediaobject]">
    <xsl:apply-templates select="node()"/>
  </xsl:template>
  
  <!-- replace synopsis with para -->
</xsl:stylesheet>
