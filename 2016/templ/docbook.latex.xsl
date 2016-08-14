<?xml version="1.0" encoding="KOI8-R"?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
  <xsl:import href="/usr/share/xml/docbook/stylesheet/db2latex/latex/docbook.xsl"/>
  <xsl:output method="text"
            encoding="KOI8-R"
            indent="no"/>
  <xsl:param name="l10n.gentext.language" select="'ru'"/>
  <xsl:param name="latex.inputenc" select="'koi8-r'"/>
  <xsl:param name="latex.documentclass.book" select="'a4paper,12pt,notitlepage,oneside,openany'"/>

  <!-- <xsl:param name="latex.document.font" select="'helvet'"/> -->

  <xsl:param name="latex.book.preamble.post">
    <xsl:text>\setlength{\parindent}{0pt}&#10;</xsl:text>
    <xsl:text>\addtolength{\parskip}{\baselineskip}&#10;</xsl:text>
    <xsl:text>\addtolength{\itemsep}{-\baselineskip}&#10;</xsl:text>
    <xsl:text><![CDATA[
\makeatletter
\def\@makechapterhead#1{%
  \vspace*{-70pt}%
  { \parindent \z@ \raggedright \normalfont
    \interlinepenalty\@M
    \Huge \thechapter \space \space \space \bfseries #1\par\nobreak
  }}
\renewcommand\section{\@startsection {section}{1}{\z@}%
  {0pt \@plus 0ex \@minus 0ex}%
  {1\parskip \@plus 0ex}%
  {\normalfont\Large\bfseries}}
\makeatother
]]></xsl:text>
  </xsl:param>
  
	<!-- inlinemediaobject <-> simplelist alignment bug fix -->
	
	<xsl:template match="simplelist|simplelist[@type='vert']" name="generate.simplelist.vert">
		<xsl:param name="environment">
			<xsl:choose>
				<xsl:when test="$latex.use.ltxtable='1' or $latex.use.longtable='1'">
					<xsl:text>longtable</xsl:text>
				</xsl:when>
				<xsl:otherwise>
					<xsl:text>tabular</xsl:text>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:param>
		<xsl:param name="cols">
			<xsl:choose>
			<xsl:when test="@columns">
				<xsl:value-of select="@columns"/>
			</xsl:when>
			<xsl:otherwise>1</xsl:otherwise>
			</xsl:choose>
		</xsl:param>
	    <xsl:call-template name="generate.simplelist.vert.2">
		<xsl:with-param name="alignment">
		    <xsl:choose>
		    <xsl:when test="name(preceding-sibling::*[position()=last()])='inlinemediaobject'">
			<xsl:value-of select="'b'"/>
		    </xsl:when>
		    <xsl:otherwise>
			<xsl:value-of select="'t'"/>
		    </xsl:otherwise>
		    </xsl:choose>
		</xsl:with-param>
		<xsl:with-param name="environment" select="$environment"/>
		<xsl:with-param name="cols" select="$cols"/>
	    </xsl:call-template>
	</xsl:template>

	<xsl:template name="generate.simplelist.vert.2">
		<xsl:param name="alignment"/>
		<xsl:param name="environment"/>
		<xsl:param name="cols"/>
		<xsl:text>&#10;</xsl:text>
		<xsl:text>\begin{</xsl:text>
		<xsl:value-of select="$environment"/>
		<!-- this is the fix: [b]! makes proper alignment with image -->
		<xsl:text>}[</xsl:text><xsl:value-of select="$alignment"/><xsl:text>]{</xsl:text>
		<xsl:call-template name="generate.simplelist.tabular.string">
			<xsl:with-param name="cols" select="$cols"/>
		</xsl:call-template>
		<xsl:text>}&#10;</xsl:text> 
		<xsl:call-template name="simplelist.vert">
			<xsl:with-param name="cols" select="$cols"/>
		</xsl:call-template>
		<xsl:text>&#10;\end{</xsl:text>
		<xsl:value-of select="$environment"/>
		<xsl:text>}&#10;</xsl:text>
	</xsl:template>
	
	<!-- correct vspace for lists -->

	<xsl:template name="compactlist.pre">
		<xsl:if test="name(preceding-sibling::node()[position()=1])='para'">
		    <xsl:text>\vspace{-1.5\baselineskip}&#10;</xsl:text>
		</xsl:if>
		<xsl:if test="@spacing='compact'">
			<xsl:if test="$latex.use.parskip=1">
				<xsl:text>&#10;\docbooktolatexnoparskip</xsl:text>
			</xsl:if>
		</xsl:if>
	</xsl:template>

	<xsl:template name="compactlist.post">
		<xsl:if test="@spacing='compact' and $latex.use.parskip=1">
			<xsl:text>\docbooktolatexrestoreparskip&#10;</xsl:text>
		</xsl:if>
		<xsl:text>\vskip -\baselineskip&#10;</xsl:text>
		<xsl:text>\vspace{-\baselineskip}&#10;</xsl:text>
		<xsl:if test="$latex.use.noindent=1">
			<xsl:text>\noindent </xsl:text>
		</xsl:if>
	</xsl:template>
	
	<!-- correct vspace for first paragraphs -->

	<xsl:template match="para|simpara">
		<xsl:if test="name(preceding-sibling::node()[position()=1])!='para'">
		    <xsl:text>\vspace{-\parskip}&#10;</xsl:text>
		</xsl:if>
		<xsl:text>&#10;</xsl:text>
		<xsl:apply-templates/>
		<xsl:text>&#10;</xsl:text>
	</xsl:template>

	<!-- correct vspace for verbatim -->
	
	<xsl:template match="address|screen|programlisting|literallayout">
		<!-- <xsl:if test="name(preceding-sibling::node()[position()=1])='para'"> -->
		    <xsl:text>\vspace{-2\baselineskip}&#10;</xsl:text>
		<!-- </xsl:if> -->

		<xsl:call-template name="verbatim.apply.templates"/>

		<xsl:text>\vskip -\baselineskip&#10;</xsl:text>
	</xsl:template>


</xsl:stylesheet>
