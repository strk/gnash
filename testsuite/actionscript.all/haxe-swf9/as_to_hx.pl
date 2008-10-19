# as_to_hx.pl - A perl script for converting actionscript2 code to Haxe.
#
#  Copyright (C) 2008 Free Software Foundation, Inc.
#
#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 3 of the License, or
#(at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program; if not, write to the Free Software
#Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
#

use strict;

my $important_string;
my $skipped = 0;
my $skipped_tests = 0;
my %vars;

while(<STDIN>){

	#Make sure important_string is clear.
	$important_string = 0;	

	#CHECK 0
	#Remove rcsid, I think makeSWF uses this, but I am not sure.
	if(index($_,"rcsid=") != $[-1){
		next;
	}

	#CHECK 1
	#Insert class definition and main() after imports.
	if(index($_,"import") != $[-1){
		print $_;
		while(<STDIN>){
			if(index($_,"import") != $[-1){
				print $_;
			}
			else{
				print "class $ARGV[0]_as{ static function main(){";
				last;
			}
		}
	}
	
	#CHECK 2
	#Check for a variable definition.
	if($_ =~ /(var\s+)(\w+)(.*)([\;\:])/){
		if(!$vars{$2}){
			$vars{$2} = 1;
			#Make sure the variable still gets declared even if this line is commented out latter.
			$important_string = declare_variable($2);
		}
#		If the type of the variable is not delcared, make it Dynamic.
		if($4 eq ';'){
			$_ = "$1$2:Dynamic $3$4\n";	
		}
	}

	#CHECK 3 - Must run after CHECK 2
	#Detect the first time a variable is used, and insert var varName; on the line above.
	if($_ =~ /^(\s*)(\w+?)(\s*=\s*.+)/){
		if(!$vars{$2}){
			print declare_variable($2) . "//FIRST OCCURENCE OF VAR $2\n";
			$vars{$2} = 1;
		}
	}

	if(index($_,"new") != $[-1){
		#Replace things like: new String; with new String();
		$_ =~ s/(new \w+)(;)/$1\(\)$2/g;
		$_ =~ s/(new \w+)(\))/$1\(\)$2/g;
		#Replace things like o = new Object(); with o = {};
		$_ =~ s/new Object\(\)/{}/g;
		#Add extra arguments when Date() is called with less than 6 args.
		if($_ =~ /new Date\(.+\)/){
			my $num_args = length(split(/,/,$_));
			if($num_args==6){
				print $_;
				next;
			}
			my $arg_string = "";
			if($num_args > 0){
				$arg_string .=",";
			}
			while($num_args<6){	
				$arg_string .= "0,";
				$num_args++;
			}
			$arg_string = substr($arg_string,0,-1);
			$_ =~ s/(new Date\(.+)(\))/$1$arg_string$2/g
		}
		#Replace calls to new String() with new String("").
		$_ =~ s/(new String\()\)/$1""\)/;

		#TODO: We can probably figure out when to use parseFloat and when to use parseInt.
		$_ =~ s/new Number\(\s*(\w+)\s*\)/Std.parseInt("$1")/g;
		
#		print $_;
#		next;
	}

	#Replace deletes:
	#ACTIONSCRIPT: delete Object.prototype.toString
	#HAXE: Reflect.deleteField(Object.prototype,toString)
	#TODO: Implement
	if(index($_,"delete") != $[-1){
		skip_line();
		next;
	}
	
	#Replace Class.hasOwnProperty(prop) with Reflect.hasField(Class,prop)
	#TODO: Make this work for other classes, if necessary
	$_ =~ s/String.hasOwnProperty\(\s*[\'\"](\w+)[\'\"]\s*\)/Reflect.hasField(String,'$1')/g;

	#Replace things like String.charCodeAt with Reflect.field(String,'charCodeAt').
	$_ =~ s/([a-zA-Z]+)\.([a-zA-Z]+)\s*([\.\)])/Reflect.field($1,'$2')$3/g;

	#Replace typeof:
	#TODO: Handle typeof and typeOf differently?
	$_ =~ s/type[Oo]f\(\s*(\S+)\s*\)/Type.typeof($1)/g;

	#Replace String() with new String("")
	$_ =~ s/(\W)String\(\)/$1new String("")/g;
	
	#Replace Class.prototype with Reflect.field(Class,'prototype')
	$_ =~ s/(\w+)\.(prototype|__proto__)/Reflect.field($1,'$2')/g;

	#There is no Function class in haxe, so we if we are trying access a property of Function
	#Like this: Function.prototype we need to replace Function with Type.getClass(function(){})
	$_ =~ s/Function/Type.getClass(function(){})/g;

	#Replace isNan(number) with Math.isNan(number)
	$_ =~ s/isNaN/Math.isNaN/g;

	#CHECK 5
	#Calls to split()
	if(index($_,"split") != $[-1){
		
		#CHECK 5.1 - Must run before CHECK 8
		#Replace calls to split that have no arguments with an array whose only member is the caller.
		$_ =~ s/(\w+)\.split\(\)/[$1]/g;

		#CHECK 5.2
		#If the delimiter is undefined, replace with an array whose only member is the caller
		$_ =~ s/(\w+)\.split\(\s*undefined\s*,.+\)/[$1]/g;

		#CHECK 5.4
		#If the limit is undefined, ignore it.
		$_ =~ s/(\w+)\.split\(\s*(\S+)\s*,\s*undefined\s*\)/$1.split($2)/g;

		#CHECK 5.3
		#Replace calls to str.split(a,b) with a==""?[]:str==""||a==null?[str]:str.split(a).slice(0,b)
		#		 str			a		b
		$_ =~ s/(\w+)\.split\((.+),\s*(\w+)\s*\)/$2==""?[]:$1==""||$2==null?[$1]:$1.split($2).slice(0,$3)/g;
		
	}

	#CHECK 8 - Must run after CHECK 5.1
	#Convert instance instanceof Class to Std.is(instance,Class)
	$_ =~ s/(\w+|\"[\w\s]+\"|\[\w+\])\s*instanceof\s*(\w+)/Std.is($1,$2)/g;

	#Replace undefined with null.
	$_ =~ s/undefined/null/g;

	#Haxe only allows one argument to String.fromCharCode(), so replace
	#String.fromCharCode(1,2) with String.fromCharCode(1) + String.fromCharCode(2)
	if($_ =~ /String.fromCharCode\(.+\)/){
		
		#TODO: Can this be combined with regex above?
		$_ =~ s/,\s*(\w+)\s*/) + String.fromCharCode($1/g;
	}
	
	#Convert calls to chr and ord.  I think these have been depreciated since SWF v5.
	$_ =~ s/chr\(\s*(\w+)\s*\)/String.fromCharCode($1)/g;
	$_ =~ s/ord\(\s*(\S+)\s*\)/$1.charCodeAt(0)/g;


	#Replace String in "for .. in" loops that iterator over String's properties with 
	#Type.getInstanceFields(String).
	$_ =~ s/(for\s*\(\s*\w+\s*in\s*)String\s*\)/$1Type\.getInstanceFields\(String\)\)/g;

	#Remove calls to String.gotcha.  I cannot find a Haxe equivilent for this.
	if($_ =~ /String.gotcha/){
		skip_line();
		next;	
	}

	#CHECK 6 - Must run before CHECK 7
	#Replace refrences to Object type with Dynamic.
	$_  =~ s/(\W)Object(\W)/$1Dynamic$2/g;

	#CHECK 7 - Must run after CHECK 6
	#Replace [object Dynamic] with [object Object]
	#This undoes some of the changes made by CHECK 6.  We need to do this because
	#even though there is no Object class in Haxe, when variables of type Dynamic are
	#compiled int byte code, they become an Object.
	$_ =~ s/\[object Dynamic\]/[object Object]/g;
	$_ =~ s/\[type Dynamic\]/[type Object]/g;

	#Remove calls to ASSetPropFlags.  I can't find a Haxe equivilent.
	if($_ =~ /ASSetPropFlags/){
		skip_line();
		next;
	}
	
	#Subtract the number of skipped tests from the total passed to Dejagnu.totals().
	$_ =~ s/Dejagnu.totals\(\s*(\w+)\s*,(.+)/Dejagnu.totals($1-$skipped_tests,$2/g;

	#Print the converted line of code.
	print $_;
}

#Close the Class and main function definitons.
print "}}\n";

print stderr "$skipped lines were skipped.\n";
print stderr "$skipped_tests tests were skipped\n";

sub skip_line{
	$skipped++;
	if($important_string){
		print $important_string;
	}
	#Keep track of the number of tests we skip.
	if($_ =~ /Dejagnu.+check/){
		$_ = "TEST DETECTED: $_";
		$skipped_tests++;
	}
	print "//$_";

}

sub declare_variable{
	
	my $var_name = $_[0];
	return "var $var_name:Dynamic='';\n"
}

sub instance_of{
	
	print stderr "INSTANCE_OF:\n";
	print stderr "ORIG=" . $_[0]."\n";
	#Convert non-alphanumeric characters to " ".
#		$working =~ s/\W/ /g;
#		print stderr $working."\n";
		my @blocks = split(/ /,$_[0],);
		my $instance_index == $[;
		foreach(@blocks){
			if($_ eq "instanceof"){
				last;
			}
			$instance_index++;
		}
		my $object = $blocks[$instance_index-1];
		my $type = $blocks[$instance_index+1];
		my $index = rindex($object,"(");
		while($index > $[-1){
			if(substr($object,$index+1,1) ne ")"){
				$object =~ s/.+\(${1}//g;
			}
			else{
				my $prev_index = rindex($object,"(",$index-1);
				$object = substr($object,$prev_index+1,1 + $index - $prev_index);
				last;
			}		
			$index = rindex($object,"(",$index-1);
		}
# = substr(0,$index+1)
		#TODO: How do I delete the characters instead of replacing it with " "
#		$object =~ s/.+\(/ /g;
		#Remove left over non-alphanumeric characters from type.
		$type =~ s/\W//g;
		my $haxe = "Std.is(" . $object . ", " . $type . ")";
		my $str_to_replace = $object." instanceof ".$type;

		print stderr "HAXE=".$haxe."\n";
		print stderr "REP=".$str_to_replace."_\n";
		print stderr "ORIG=" . $_[0] ."\n";
		print stderr "REP_INDEX=" . index($_[0],$str_to_replace) . "\n";
		$_[0] =~ s/\Q$str_to_replace\E/$haxe/g;
		print stderr "FINAL=" . $_[0] . "\n";
		return $_[0];
	}