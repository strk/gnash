createTextField("out",300000,0,0,400,400);

// FIXME: _global object isn't recognized
//_global.xtrace = function (msg)

xtrace = function (msg) 
{
	_level0.out.text += msg+"\n";
};

dumpObject = function(obj, indent)
{
   var s = '';
   //if ( typeof(obj) == 'object' )

   if ( indent == undefined ) indent = 0;

   for (var i in obj)
   {
      var value = obj[i];
                        for (var j=0; j<indent; j++) s += ' ';
                        if ( typeof(value) == 'object' )
                        {
                                s += i+" (object):\n";
                                s += dumpObject(value, ++indent);
                        }
                        else
                        {
        s += i+'='+value+' ('+typeof(value)+')\n';
                        }
   }
   return s;
};

xtrace("Xtrace working");
