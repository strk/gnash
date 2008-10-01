
use strict;

my $skipped = 0;

while(<STDIN>){

	#Remove rcsid, I think makeSWF uses this, but I am not sure.
	if(index($_,"rcsid=") != $[-1){
		print "//".$_;
		next;
	}
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
	if(index($_,"new") != $[-1){
		#Replace things like: new String; with new String();
		$_ =~ s/(new \w+)(;)/$1\(\)$2/g;
		$_ =~ s/(new \w+)(\))/$1\(\)$2/g;
		#Replace things like o = new Object; with o = "";
		$_ =~ s/new Object\(\)/""/g;
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
		$skipped++;
		print "//".$_;
#		print instance_of($_);	
		next;
	}
	
	#Replace deletes:
	#ACTIONSCRIPT: delete Object.prototype.toString
	#HAXE: Reflect.deleteField(Object.prototype,toString)
	#TODO: Implement
	if(index($_,"delete") != $[-1){
		$skipped++;
		print "//".$_;
		next;
	}
	#Replace instanceof:
	#TODO: Implement
	if(index($_,"typeof") != $[-1){
		$skipped++;
		print "//".$_;
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
		print "//".$_;
		next;
	}
	if(index($_,"__proto__") != $[-1){
		print "//".$_;
		next;
	}
	if(index($_,"prototype") != $[-1){
		print "//".$_;
		next
	}
	if(index($_,"isNaN") != $[-1){
#		$_ =~ s/(isNaN)/Math\.$1/g;
		print "//".$_;
		next;
	}
	if(index($_,"split") != $[-1){
		
		#Replace calls to split whose second argument is 0 with an empty array.
		$_ =~ s/\w+\.split\(.+, +0\)/[]/g;
		
		#Remove calls to split that have more than one argument.
		if($_ =~ /\.split\(.+,.+\)/){
			print "//".$_;
			next;
		}
		#Replace calls to split that have no arguments with an array whose only member is the caller.
		$_ =~ s/(\w+)\.split\(\)/\[$1\]/g;
#		print $_;
#		next;
	}
	if(index($_,"length") != $[-1){
		#Remove attemps to set strings length property.  Haxe compliler does not allow this.
		if($_ =~ /\w+\.length.*=.+;/){
			print "//".$_;
			next;	
		}
	}
	#Ignore calls to concat, I cannot find the equivilent haxe function.
	if(index($_,"concat") != $[-1){
		print "//".$_;
		next;
	}
	#Remove lines that contain things like this:
	# o = {}
	#I am not sure what this means.
	if($_ =~ /=.+\{\}/){
		print "//".$_;
		next; 
	}
	#Remove return calls(TEMP).
	if(index($_,"return") != $[-1){
		print "//".$_;
		next;
	}
	if($_ =~ /Number/){
		print "//".$_;
		next;
	}
	#Remove calls to hasOwnProperty.  I can't find a Haxe equivilent for this.
	if($_=~ /hasOwnProperty/){
		print "//".$_;
		next;
	}
	if($_ =~ /indexOf/){
		print "//".$_;
		next;
	}
	#Remove references to the undefined value. I can't find a Haxe equivilent for this.
	if($_ =~ /undefined/){
		print "//".$_;
		next;
	}
	#Remove calls to fromCharCode.  Haxe only alows one argument to this function.
	if($_ =~ /String.fromCharCode\(.+\)/){
		print "//".$_;
		next;
	}
	#Replace calls to slice with substr.
	$_ =~ s/(\.)slice(\(.+\))/$1substr$2/g;
	
	#Remove calls to call function.  I haven't found a Haxe equivilent for this.
	if($_ =~ /\.call\(.+\)/){
		print "//".$_;
		next;
	}
	
	#Remove calls to chr and ord.  I think these have been depreciated since SWF v5.
	if($_ =~ /ord\(.+\)/ || $_ =~ /chr\(.+\)/){
		print "//".$_;
		next;
	}
	#Replace String in "for .. in" loops that iterator over String's properties with Reflect.fields(String).
	$_ =~ s/(for\s*\(\s*\w+\s*in\s*)String\s*\)/$1Reflect\.fields\(String\)\)/g;

	#Remove calls to String.gotcha.  I cannot find a Haxe equivilent for this.
	if($_ =~ /String.gotcha/){
		print "//".$_;
		next;	
	}
	#Nothing special found.
	print $_;
}

print "}}";

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