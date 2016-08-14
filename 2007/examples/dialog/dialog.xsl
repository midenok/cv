<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:strip-space elements="*"/>
    <xsl:output method="xml" indent="yes"/>

<!---=================================  TO Conversion ==========================================-->

    <xsl:template match="/gff/struct">
        <xsl:element name="nwndialog">
            <xsl:attribute name="dlg">
                <xsl:value-of select="../@name"/>
            </xsl:attribute>
            <xsl:attribute name="dlgver">
                <xsl:value-of select="../@version"/>
            </xsl:attribute>
            <xsl:apply-templates mode="root"/>
        </xsl:element>
    </xsl:template>

    <xsl:template mode="root" match="element[@name='DelayEntry']">
        <xsl:if test="@value!=0">
            <xsl:attribute name="entrydelay">
                <xsl:value-of select="@value"/>
            </xsl:attribute>
        </xsl:if>
    </xsl:template>

    <xsl:template mode="root" match="element[@name='DelayReply']">
        <xsl:if test="@value!=0">
            <xsl:attribute name="replydelay">
                <xsl:value-of select="@value"/>
            </xsl:attribute>
        </xsl:if>
    </xsl:template>

    <xsl:template mode="root" match="element[@name='NumWords']">
        <xsl:if test="@value!=0">
            <xsl:attribute name="words">
                <xsl:value-of select="@value"/>
            </xsl:attribute>
        </xsl:if>
    </xsl:template>

    <xsl:template mode="root" match="element[@name='EndConversation']">
        <xsl:if test="@value!=''">
            <xsl:attribute name="convend">
                <xsl:value-of select="@value"/>
            </xsl:attribute>
        </xsl:if>
    </xsl:template>

    <xsl:template mode="root" match="element[@name='EndConverAbort']">
        <xsl:if test="@value!=''">
            <xsl:attribute name="convabort">
                <xsl:value-of select="@value"/>
            </xsl:attribute>
        </xsl:if>
    </xsl:template>

    <xsl:template mode="root" match="element[@name='PreventZoomIn']">
        <xsl:if test="@value!=0">
            <xsl:attribute name="nozoom">
                <xsl:value-of select="@value"/>
            </xsl:attribute>
        </xsl:if>
    </xsl:template>

    <xsl:template mode="options" match="element[@name='Script']">
        <xsl:if test="@value!=''">
            <xsl:attribute name="action">
                <xsl:value-of select="@value"/>
            </xsl:attribute>
        </xsl:if>
    </xsl:template>

    <xsl:template mode="options" match="element[@name='Animation']">
        <xsl:if test="@value!=0">
            <xsl:attribute name="animation">
                <xsl:value-of select="@value"/>
            </xsl:attribute>
        </xsl:if>
    </xsl:template>

    <xsl:template mode="options" match="element[@name='Speaker']">
        <xsl:if test="@value!=''">
            <xsl:attribute name="speaker">
                <xsl:value-of select="@value"/>
            </xsl:attribute>
        </xsl:if>
    </xsl:template>

    <xsl:template mode="options" match="element[@name='AnimLoop']">
        <xsl:if test="@value!=1">
            <xsl:attribute name="animloop">
                <xsl:value-of select="@value"/>
            </xsl:attribute>
        </xsl:if>
    </xsl:template>

    <xsl:template mode="options" match="element[@name='Delay']">
        <xsl:if test="@value!=-1">
            <xsl:attribute name="delay">
                <xsl:value-of select="@value"/>
            </xsl:attribute>
        </xsl:if>
    </xsl:template>

    <xsl:template mode="options" match="element[@name='Comment']">
        <xsl:if test="@value!='' or value">
            <xsl:attribute name="comment">
                <xsl:value-of select="concat(@value, value)"/>
            </xsl:attribute>
        </xsl:if>
    </xsl:template>

    <xsl:template mode="options" match="element[@name='Sound']">
        <xsl:if test="@value!=''">
            <xsl:attribute name="sound">
                <xsl:value-of select="@value"/>
            </xsl:attribute>
        </xsl:if>
    </xsl:template>

    <xsl:template mode="options" match="element[@name='Quest']">
        <xsl:if test="@value!=''">
            <xsl:attribute name="quest">
                <xsl:value-of select="@value"/>
            </xsl:attribute>
        </xsl:if>
    </xsl:template>

    <xsl:template mode="root" match="element[@name='StartingList']">
        <xsl:apply-templates mode="list">
            <xsl:with-param name="side" select="1"/>
        </xsl:apply-templates>
    </xsl:template>

    <xsl:template mode="root" match="text()"/>
    <xsl:template mode="side" match="text()"/>

    <xsl:template mode="side" match="element[@name='Text']">
        <xsl:param name="side"/>
        <xsl:param name="cond"/>
        <xsl:element name="{substring('er', $side+1, 1)}">
            <xsl:attribute name="id">
                <xsl:value-of select="../@id"/>
            </xsl:attribute>
            <xsl:if test="@value != -1">
                <xsl:attribute name="strref">
                    <xsl:value-of select="@value"/>
                </xsl:attribute>
            </xsl:if>
            <xsl:if test="localString">
                <xsl:attribute name="str">
                    <xsl:value-of select="localString[1]/@value"/>
                </xsl:attribute>
            </xsl:if>
            <xsl:if test="$cond != ''">
                <xsl:attribute name="if">
                    <xsl:value-of select="$cond"/>
                </xsl:attribute>
            </xsl:if>
            <xsl:apply-templates mode="options" select=".."/>
            <xsl:apply-templates mode="list" select="../element[@name=substring('RepliesListEntriesList', $side*11+1, 11)]">
                <xsl:with-param name="side" select="$side"/>
            </xsl:apply-templates>
        </xsl:element>
        <xsl:value-of select="' '"/>
        <xsl:comment><xsl:value-of select="concat(' ', ../@id, ' ')"/></xsl:comment>
    </xsl:template>

    <xsl:template mode="list" match="struct">
        <xsl:param name="side"/>
        <xsl:choose>
        <xsl:when test="not(element[@name='IsChild']) or element[@name='IsChild']/@value = 0">
            <xsl:variable name="id" select="element[@name='Index']/@value"/>
            <xsl:variable name="list" select="substring('ReplyListEntryList', $side*9+1, 9)"/>
            <xsl:apply-templates mode="side" select="/gff/struct/element[@name=$list]/struct[@id=$id]">
                <xsl:with-param name="side" select="1-$side"/>
                <xsl:with-param name="cond" select="element[@name='Active']/@value"/>
            </xsl:apply-templates>
        </xsl:when>
        <xsl:otherwise>
            <xsl:element name="{substring('lrle', $side*2+1, 2)}">
                <xsl:attribute name="href">
                    <xsl:value-of select="element[@name='Index']/@value"/>
                </xsl:attribute>
                <xsl:if test="element[@name='Active']/@value != ''">
                    <xsl:attribute name="if">
                        <xsl:value-of select="element[@name='Active']/@value"/>
                    </xsl:attribute>
                </xsl:if>
            </xsl:element>
        </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

<!---=================================  FROM Conversion ==========================================-->

    <xsl:template match="/nwndialog">
        <nwndialog-p2>
            <xsl:copy-of select="@*"/>
            <xsl:apply-templates mode="p1"/>
        </nwndialog-p2>
    </xsl:template>
    
    <xsl:template mode="p1" match="include">
        <xsl:apply-templates mode="p1" select="document(@file)"/>
    </xsl:template>
    
    <xsl:template mode="p1" match="node()|@*">
        <xsl:copy>
            <xsl:apply-templates mode="p1" select="node()|@*"/>
        </xsl:copy>
    </xsl:template>

    <xsl:template match="/nwndialog-p2">
        <gff name="{@dlg}" type="DLG " version="{@dlgver}">
            <struct id="-1">
                <xsl:choose><xsl:when test="@entrydelay">
                    <element name="DelayEntry" type="4" value="{@entrydelay}"/>
                </xsl:when><xsl:otherwise>
                    <element name="DelayEntry" type="4" value="0"/>
                </xsl:otherwise></xsl:choose>

                <xsl:choose><xsl:when test="@replydelay">
                    <element name="DelayReply" type="4" value="{@replydelay}"/>
                </xsl:when><xsl:otherwise>
                    <element name="DelayReply" type="4" value="0"/>
                </xsl:otherwise></xsl:choose>

                <xsl:choose><xsl:when test="@words">
                    <element name="NumWords" type="4" value="{@words}"/>
                </xsl:when><xsl:otherwise>
                    <element name="NumWords" type="4" value="0"/>
                </xsl:otherwise></xsl:choose>
                
                <xsl:choose><xsl:when test="@convend">
                    <element name="EndConversation" type="11" value="{@convend}"/>
                </xsl:when><xsl:otherwise>
                    <element name="EndConversation" type="11" value=""/>
                </xsl:otherwise></xsl:choose>

                <xsl:choose><xsl:when test="@convabort">
                    <element name="EndConverAbort" type="11" value="{@convabort}"/>
                </xsl:when><xsl:otherwise>
                    <element name="EndConverAbort" type="11" value=""/>
                </xsl:otherwise></xsl:choose>
                
                <xsl:choose><xsl:when test="@nozoom">
                    <element name="PreventZoomIn" type="0" value="{@nozoom}"/>
                </xsl:when><xsl:otherwise>
                    <element name="PreventZoomIn" type="0" value="0"/>
                </xsl:otherwise></xsl:choose>
                <element name="EntryList" type="15">
                    <xsl:apply-templates mode="genlist" select="descendant::e">
                        <xsl:with-param name="side" select="0"/>
                    </xsl:apply-templates>
                </element>
                <element name="ReplyList" type="15">
                    <xsl:apply-templates mode="genlist" select="descendant::r">
                        <xsl:with-param name="side" select="1"/>
                    </xsl:apply-templates>
                </element>
                <element name="StartingList" type="15">
                    <xsl:apply-templates mode="listitem">
                        <xsl:with-param name="side" select="1"/>
                        <xsl:with-param name="links" select="0"/>
                    </xsl:apply-templates>
                </element>
            </struct>
        </gff>
    </xsl:template>
    
    <xsl:template mode="genlist" match="*">
        <xsl:param name="side"/>
        <struct id="{position()-1}">
            <xsl:if test="$side=0">
                <xsl:choose><xsl:when test="@speaker">
                    <element name="Speaker" type="10" value="{@speaker}"/>
                </xsl:when><xsl:otherwise>
                    <element name="Speaker" type="10" value=""/>
                </xsl:otherwise></xsl:choose>
            </xsl:if>
            <xsl:choose><xsl:when test="@animation">
                <element name="Animation" type="4" value="{@animation}"/>
            </xsl:when><xsl:otherwise>
                <element name="Animation" type="4" value="0"/>
            </xsl:otherwise></xsl:choose>
            <xsl:choose><xsl:when test="@animloop">
                <element name="AnimLoop" type="0" value="{@animloop}"/>
            </xsl:when><xsl:otherwise>
                <element name="AnimLoop" type="0" value="1"/>
            </xsl:otherwise></xsl:choose>
            <element name="Text" type="12">
                <xsl:attribute name="value">
                    <xsl:choose><xsl:when test="@strref">
                        <xsl:value-of select="@strref"/>
                    </xsl:when><xsl:otherwise>
                        <xsl:value-of select="-1"/>
                    </xsl:otherwise></xsl:choose>
                </xsl:attribute>
                <xsl:if test="@str">
                    <localString languageId="0" value="{@str}"/>
                </xsl:if>
            </element>
            <xsl:choose><xsl:when test="@action">
                <element name="Script" type="11" value="{@action}"/>
            </xsl:when><xsl:otherwise>
                <element name="Script" type="11" value=""/>
            </xsl:otherwise></xsl:choose>
            <xsl:choose><xsl:when test="@delay">
                <element name="Delay" type="4" value="{@delay}"/>
            </xsl:when><xsl:otherwise>
                <element name="Delay" type="4" value="-1"/>
            </xsl:otherwise></xsl:choose>
            <xsl:choose><xsl:when test="@comment">
                <element name="Comment" type="10" value="{@comment}"/>
            </xsl:when><xsl:otherwise>
                <element name="Comment" type="10" value=""/>
            </xsl:otherwise></xsl:choose>
            <xsl:choose><xsl:when test="@sound">
                <element name="Sound" type="11" value="{@sound}"/>
            </xsl:when><xsl:otherwise>
                <element name="Sound" type="11" value=""/>
            </xsl:otherwise></xsl:choose>
            <xsl:choose><xsl:when test="@quest">
                <element name="Quest" type="10" value="{@quest}"/>
            </xsl:when><xsl:otherwise>
                <element name="Quest" type="10" value=""/>
            </xsl:otherwise></xsl:choose>
            <element name="{substring('RepliesListEntriesList', $side*11+1, 11)}" type="15">
                <xsl:apply-templates mode="listitem">
                    <xsl:with-param name="side" select="$side"/>
                </xsl:apply-templates>
            </element>
        </struct>
    </xsl:template>
    
    <xsl:template mode="listitem" match="*">
        <xsl:param name="side"/>
        <xsl:param name="links" select="1"/>
        <xsl:variable name="islink" select="number(substring(local-name(), 1, 1)='l')"/>
        <xsl:variable name="id">
            <xsl:choose><xsl:when test="$islink">
                <xsl:value-of select="@href"/>
            </xsl:when><xsl:otherwise>
                <xsl:value-of select="@id"/>
            </xsl:otherwise></xsl:choose>
        </xsl:variable>
        <struct id="{position()-1}">
            <element name="Index" type="4">
                <xsl:attribute name="value">
                    <xsl:choose><xsl:when test="$side=0">
                        <xsl:apply-templates mode="getpos" select="/descendant::r">
                            <xsl:with-param name="id" select="$id"/>
                        </xsl:apply-templates>
                    </xsl:when><xsl:otherwise>
                        <xsl:apply-templates mode="getpos" select="/descendant::e">
                            <xsl:with-param name="id" select="$id"/>
                        </xsl:apply-templates>
                    </xsl:otherwise></xsl:choose>
                </xsl:attribute>
            </element>
            <element name="Active" type="11" value="{@if}"/>
            <xsl:if test="$links">
                <element name="IsChild" type="0" value="{$islink}"/>
            </xsl:if>
        </struct>
    </xsl:template>
    
    <xsl:template mode="getpos" match="*">
        <xsl:param name="id"/>
        <xsl:if test="@id=$id">
            <xsl:value-of select="position()-1"/>
        </xsl:if>
    </xsl:template>
</xsl:stylesheet>

