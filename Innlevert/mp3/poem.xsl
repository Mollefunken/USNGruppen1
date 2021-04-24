<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/">
  <table>
    <tr>
      <th style="text-align:left">ID</th>
      <th style="text-align:left">Name</th>
    </tr>
    <xsl:for-each select="root/Dikt/dikt">
    <tr>
      <td><xsl:value-of select="diktID"/></td>
      <td><xsl:value-of select="dikt"/></td>
    </tr>
    </xsl:for-each>
  </table>
</xsl:template>
</xsl:stylesheet>
