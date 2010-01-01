#!/bin/sh

# 
#   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

# This script generates templates for documenting an ActionScript
# class. It depends on the NOTES file for data. It takes a single
# argument, which is the name of the class, like "./gen.sh Camera".
#
# This script is only of use to developers, so it's "as is". Your
# mileage may vary.
asname=$1
newname=`echo ${asname} | tr '[A-Z]' '[a-z]'`
outname=${newname}.xml

methods=`grep "$asname\..*()" NOTES | sed -e 's/XML\.//g'`
props=`grep "$asname\." NOTES | grep -v "()" | sed -e 's/XML\.//g'`

#echo $methods
#echo $props

rm -f ${outname}
cat <<EOF>>${outname}
<sect4 id="as${newname}">
  <title>${asname} ActionScript Class</title>

  <para>
    This class implements an ${asname} object.
  </para>

    <sect5 id="${newname}methods">
	<title>The Methods of the ${i} Class</title>
	<para>
	    <variablelist>
EOF


for i in $methods; do

cat <<EOF>>${outname}

		<varlistentry>
		    <term>$i</term>
		    <listitem>
		    <para>
		    </para>
		    </listitem>
	    </varlistentry>
EOF
done

cat <<EOF>>${outname}
	</variablelist>
	</para>
    </sect5>
EOF

cat <<EOF>>${outname}
  <sect5 id="${newname}props">
    <title>The Properties of the ${asname} Class</title>
    
    <para>
      <variablelist>
EOF

for i in $props; do

cat <<EOF>>${outname}

	<varlistentry>
	  <term>$i</term>
	  <listitem>
	    <para>
	    </para>
	  </listitem>
	</varlistentry>
EOF
done

cat <<EOF>>${outname}

      </variablelist>
    </para>
  </sect5>

  <sect5 id="${newname}conf">
    <title>${asname} Class Conformance</title>
    
    <para>
      <informaltable frame="all">
	<?dbhtml table-width="75%" ?>
	<tgroup cols="2">
	  <thead>
	    <row>
	      <entry valign="top">
		<para>Class Name</para>
	      </entry>
	      <entry valign="top">
		<para>Conformance</para>
	      </entry>
	    </row>
	  </thead>
	  <tbody>
EOF

for i in $methods; do

cat <<EOF>>${outname}
	    <row>
	      <entry valign="top" align="left">
		<para>$i</para>
	      </entry>
	      <entry valign="top" align="center">
		<para>
		  This method has an unknown status.
		</para>
	      </entry>
	    </row>
EOF
done

for i in $props; do

cat <<EOF>>${outname}
	    <row>
	      <entry valign="top" align="left">
		<para>$i</para>
	      </entry>
	      <entry valign="top" align="center">
		<para>
		  This property has an unknown status.
		</para>
	      </entry>
	    </row>
EOF
done

cat <<EOF>>${outname}
	  </tbody>
	</tgroup>
      </informaltable>
    </para>
  </sect5>  
</sect4>
EOF
