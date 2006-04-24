#!/bin/sh

# This is a simple script that takes the output of "cvs diff".
# and produces a changelog entry template with the names of the files,
# the function or method name, and whether lines were added, removed,
# or changed. You can then easily fill in the raw data with proper
# ChangeLog entries.

# Please do not file bug reports on this script, it is unsupported. I
# just checked in it incase other developers find it useful.

infile=Cvs.diff
outfile=entry.txt
declare -a files

# Format the header line
name=`grep $USER /etc/passwd | cut -d : -f 5 | sed -e 's:,::g'`
now=`date -u "+%Y-%m-%d"`
email="<$USER@$HOST>"
header="$now  $name   $email"

# get the files
count=`grep -c "^RCS" ${infile}`

entries=`grep "^RCS" ${infile} | sed -e 's/RCS file: //' -e 's:,v::'`
idx=0
for i in ${entries}; do
  lines[$idx]=`grep -n "$i" ${infile} | cut -d : -f 1`
  files[$idx]=`echo $i| cut -d / -f 5-9`
  idx=`expr ${idx}+1`
done
# the last line is the size of the file
lines[$idx]=`cat ${infile} | wc -l`

# Start the output file
rm -f $outfile
echo $header > $outfile
echo >> $outfile

# add entries for each file
idx=0
#set noglob
while test $idx != $count; do
#  echo "	* ${files[idx]}: " >> $outfile
  filename=`eval echo ${files[idx]}`
  out="\t\052 ${filename}: "
  start=${lines[idx]}
  idx=`expr ${idx} + 1`
  end=${lines[idx]}
  # this gets the numbers for the lines that changed
  chlines=`sed -n -e "${start},${end}p" ${infile} | grep "@@" | sed -e 's:@@::g'`
#  echo $chlines | sed -e 's/ @@/)/g' -e 's/@@ /(/g'
#  echo $chlines
  for i in $chlines; do
    char=`echo $i | cut -b 1`
    if test x"${char}" = x"-"; then
      firstline=`echo $i | cut -f 1 -d , | cut -b 2-100`
      firstcount=`echo $i | cut -f 2 -d ,`
    fi
    if test x"${char}" = x"+"; then
      rm -f .tmp
      # extract the function definitions
      grep -n "::.*(.*)" $filename 2>/dev/null | egrep -v ';|>|\.|while|=' > .tmp
      # get just the line numbers of the definitions
      hits=`sed -e 's/:.*$//' .tmp`
      # the the definition for this entry
      previous=0
      func=""
      for k in $hits; do
        if test $k -gt $firstline; then
	  func=`grep $previous .tmp | cut -d : -f 2-20`
	  if test x"$func" != x; then
	    func=`echo $func | sed -e 's:(.*)::' -e 's:,::'`
	    func="(${func}) "
	  fi
	  break
	else
  	  previous=$k
	fi
      done
      # extract the line number and lines changed fields.
      secondline=`echo $i | cut -f 1 -d , | cut -b 2-100`
      secondcount=`echo $i | cut -f 2 -d ,`
      endline=`expr $firstline - $firstcount + $secondcount`
      if test $firstcount -eq $secondcount; then
        out="$out ${func} Changed lines $firstline-$endline."
        continue
      fi
      if test $firstcount -lt $secondcount; then
        out="$out ${func} Added lines $firstline-$endline."
        continue
      fi
      if test $firstcount -gt $secondcount; then
        out="$out ${func} Removed lines $firstline-$endline."
	continue
      fi
    fi
  done
  echo -e $out | fmt >> $outfile
done

# put a blank line on the end
echo >> $outfile

