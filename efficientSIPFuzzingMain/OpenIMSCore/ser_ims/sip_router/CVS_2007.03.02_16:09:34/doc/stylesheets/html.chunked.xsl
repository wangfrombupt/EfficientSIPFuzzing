<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:exsl="http://exslt.org/common"
		version="1.0"
                exclude-result-prefixes="exsl">

<!-- ********************************************************************
     $Id: html.chunked.xsl 165 2007-03-02 15:15:46Z vingarzan $
     ********************************************************************

     This file is part of the XSL DocBook Stylesheet distribution.
     See ../README or http://nwalsh.com/docbook/xsl/ for copyright
     and other information.

     ******************************************************************** -->

<!-- ==================================================================== -->

<xsl:import href="http://docbook.sourceforge.net/release/xsl/current/html/docbook.xsl"/>
<xsl:import href="http://docbook.sourceforge.net/release/xsl/current/html/chunk-common.xsl"/>
<xsl:include href="http://docbook.sourceforge.net/release/xsl/current/html/manifest.xsl"/>

<!-- Why is chunk-code now xsl:included?

Suppose you want to customize *both* the chunking algorithm used *and* the
presentation of some elements that may be chunks. In order to do that, you
must get the order of imports "just right". The answer is to make your own
copy of this file, where you replace the initial import of "docbook.xsl"
with an import of your own base.xsl (that does its own import of docbook.xsl).

Put the templates for changing the presentation of elements in your base.xsl.

Put the templates that control chunking after the include of chunk-code.xsl.

Voila! (Man I hope we can do this better in XSLT 2.0)

-->

<xsl:include href="http://docbook.sourceforge.net/release/xsl/current/html/chunk-code.xsl"/>

</xsl:stylesheet>
