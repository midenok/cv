<?xml version="1.0" encoding="UTF8"?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
  <xsl:import href="/usr/share/xml/docbook/stylesheet/docbook-xsl/html/docbook.xsl"/>
  <!--<xsl:import href="/usr/share/sgml/docbook/xsl-stylesheets/html/highlight.xsl"/>-->
  <xsl:output method="html"
          encoding="UTF8"
          indent="no"/>
  <xsl:param name="l10n.gentext.language" select="'ru'"/>
  <xsl:param name="html.stylesheet" select="'style.css'"/>
  <xsl:param name="section.autolabel" select="1"/>
  <!--<xsl:param name="highlight.source" select="1"/>-->
  <xsl:attribute-set name="monospace.verbatim.properties">
    <xsl:attribute name="font-size">6pt</xsl:attribute>
  </xsl:attribute-set>
</xsl:stylesheet>
