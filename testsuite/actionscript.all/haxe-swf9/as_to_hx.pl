
use strict;

my $important_string;
my $skipped = 0;
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
	
	#CHECK 4 - Must run before CHECK 5.3
	#Skip calls to slice there isn't an exact Haxe equivalent to this.
	if($_ =~ /\.slice\(.+\)/){
		skip_line();
		next;
	}

	if(index($_,"new") != $[-1){
		#Replace things like: new String; with new String();
		$_ =~ s/(new \w+)(;)/$1\(\)$2/g;
		$_ =~ s/(new \w+)(\))/$1\(\)$2/g;
		#Replace things like o = new Object(); with o = {};
		$_ =~ s/\s*(\w+)\s*=\s*new Object\(\)/$1={}/g;
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
		
#		print $_;
#		next;
	}
	#Convert instance of expressions.
	#TODO: This only works some of the time.  Skipping these for now.
	#BROKEN: 
	#Dejagnu.check(! "literal string" instanceof String, "! \"literal string\" instanceof String"+' '+' ['+"String.as"+':'+1056 +']');
	if(index($_,"instanceof") != $[-1){
		skip_line();
#		print instance_of($_);	
		next;
	}
	
	#Replace deletes:
	#ACTIONSCRIPT: delete Object.prototype.toString
	#HAXE: Reflect.deleteField(Object.prototype,toString)
	#TODO: Implement
	if(index($_,"delete") != $[-1){
		skip_line();
		next;
	}
	#Replace typeof:
	#TODO: Figure out the difference between typeof and typeOf
	if($_ =~ /type[Oo]f/ ){
		skip_line();
		next;
	}
	if(index($_,"String") != $[-1){
#		$_ =~ s/new +String/Std\.string/g;
#		$_ =~ s/String/Std\.string/g;
		#Don't ignore new String()
		#Fix this.
		if($_ =~ /new String\(.+\)/){
			
		}
		else{
#			print "//".$_;
#			next;
		}
	}
	if(index($_,"substring") != $[-1){
		skip_line();
		next;
	}
	if(index($_,"__proto__") != $[-1){
		skip_line();
		next;
	}
	if(index($_,"prototype") != $[-1){
		$_ =~ s/(\w+)\.(prototype)/Reflect.field($1,\'$2\')/g;
#		skip_line();
#		next
	}
	if(index($_,"isNaN") != $[-1){
#		$_ =~ s/(isNaN)/Math\.$1/g;
		skip_line();
		next;
	}
	#CHECK 5
	#Calls to split()
	if(index($_,"split") != $[-1){
		
		#CHECK 5.1
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
	if(index($_,"length") != $[-1){
		#Remove attemps to set strings length property.  Haxe compliler does not allow this.
		if($_ =~ /\w+\.length.*=.+;/){
			skip_line();
			next;	
		}
	}
	#Ignore calls to concat, I cannot find the equivilent haxe function.
	if(index($_,"concat") != $[-1){
		skip_line();
		next;
	}

	if($_ =~ /Number/){
		skip_line();
		next;
	}
	#Remove calls to hasOwnProperty.  I can't find a Haxe equivilent for this.
	if($_=~ /hasOwnProperty/){
		skip_line();
		next;
	}
	#Remove calls to String.indexOf that have more than one argument
	if($_ =~ /indexOf\(\s*[\"\w]\w*[\"\w],.+\)/){
		skip_line();
		next;
	}

	#Replace undefined with null.
	$_ =~ s/undefined/null/g;

	#Haxe only allows one argument to String.fromCharCode(), so replace
	#String.fromCharCode(1,2) with String.fromCharCode(1) + String.fromCharCode(2)
	if($_ =~ /String.fromCharCode\(.+\)/){
		
		#TODO: Can this be combined with regex above?
		$_ =~ s/,\s*(\w+)\s*/) + String.fromCharCode($1/g;
	}
	
	#Remove calls to call function.  I haven't found a Haxe equivilent for this.
	if($_ =~ /\.call\(.+\)/){
		skip_line();
		next;
	}
	
	#Remove calls to chr and ord.  I think these have been depreciated since SWF v5.
	if($_ =~ /ord\(.+\)/ || $_ =~ /chr\(.+\)/){
		skip_line();
		next;
	}
	#Replace String in "for .. in" loops that iterator over String's properties with Reflect.fields(String).
	$_ =~ s/(for\s*\(\s*\w+\s*in\s*)String\s*\)/$1Reflect\.fields\(String\)\)/g;

	#Remove calls to String.gotcha.  I cannot find a Haxe equivilent for this.
	if($_ =~ /String.gotcha/){
		skip_line();
		next;	
	}

	#Replace refrences to Object type with Dynamic.
	$_  =~ s/(\W)Object(\W)/$1Dynamic$2/g;	

	#Remove calls to ASSetPropFlags.  I can't find a Haxe equivilent.
	if($_ =~ /ASSetPropFlags/){
		skip_line();
		next;
	}
	#Print the converted line of code.
	print $_;
}

#Close the Class and main function definitons.
print "}}\n";

print stderr "$skipped lines were skipped.\n";

sub skip_line{
	$skipped++;
	if($important_string){
		print $important_string;
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