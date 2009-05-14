import flash.Lib;
import flash.text.TextField;
class Dejagnu {
    static var passed = 0;
    static var failed = 0;
    static var xpassed = 0;
    static var xfailed = 0;
    static var untest = 0;
    static var unresolve = 0;
    static var inithack = init();
    static var tf:flash.text.TextField;
    static function init() {
 tf = new flash.text.TextField();
 tf.autoSize = flash.text.TextFieldAutoSize.LEFT;
 flash.Lib.current.addChild(tf);
 return null;
    }
    static function fail (why) {
        failed++;
        var msg = 'FAILED: '+why;
        xtrace(msg);
    }
    static function xfail(why) {
        xfailed++;
        var msg = 'XFAILED: '+why;
        xtrace(msg);
    }
    static function pass(why) {
        passed++;
        var msg = 'PASSED: '+why;
        flash.Lib.trace(msg);
    }
    static function xpass(why) {
        xpassed++;
        var msg = 'XPASSED: '+why;
        flash.Lib.trace(msg);
    }
    static function testcount() {
        var c = 0;
        if ( passed > 0 ) c += passed;
        if ( failed > 0 ) c += failed;
        if ( xpassed > 0 ) c += xpassed;
        if ( xfailed > 0 ) c += xfailed;
        return c;
    }
    static function printtotals() {
        xtrace('#passed: '+ passed);
        xtrace('#failed: '+ failed);
        if ( xpassed > 0 ) {
            xtrace('#unexpected successes: '+ xpassed);
        }
        if ( xfailed > 0 ) {
            xtrace('#expected failures: '+ xfailed);
        }
  xtrace('#total tests run: '+ testcount());
    }
    static public function totals(exp:Dynamic, msg:Dynamic) {
        var obt = testcount();
        if ( exp != null && obt != exp ) {
            fail('Test run '+obt+' (expected '+exp+') '+msg);
        } else {
            pass('Test run '+obt+' '+msg);
        }
    }
    static function xtotals(exp:Dynamic, msg:Dynamic) {
        var obt = testcount();
        if ( exp != null && obt != exp ) {
            xfail('Test run '+obt+' (expected '+exp+') ['+msg+']');
        } else {
            xpass('Test run '+obt+' ['+msg+']');
        }
    }
    static public function check_equals(obt:Dynamic, exp:Dynamic, msg) {
        if(msg == null) msg = "";
        if ( obt == exp )
            pass(obt+' == '+exp+' '+msg);
        else
            fail('expected: "'+exp+'" , obtained: "'+obt+'" '+msg);
    }
 static public function xcheck_equals(obt:Dynamic, exp:Dynamic, msg)
 {
  if(msg == null) msg = "";
  if ( obt == exp )
  {
   xpass(obt+' == '+exp+' '+msg);
  }
  else
  {
   xfail('expected: '+exp+' , obtained: '+obt+" "+msg);
  }
 }
 static public function check(a : Dynamic, msg)
 {
  if ( a )
  {
   if ( msg != null ) pass(msg);
   else pass(a);
  }
  else
  {
   if ( msg != null ) fail(msg);
   else fail(a);
  }
 }
 static public function xcheck(a : Dynamic, msg)
 {
  if ( a )
  {
   if ( msg != null ) xpass(msg);
   else xpass(a);
  }
  else
  {
   if ( msg != null ) xfail(msg);
   else xfail(a);
  }
 }
    static public function note(msg) {
        xtrace(msg);
    }
    static function xtrace(msg) {
        tf.text += msg + "\n";
        flash.Lib.trace(msg);
    }
    static function untested(msg) {
        flash.Lib.trace("UNTESTED: "+msg);
    }
    static function unresolved(msg) {
        flash.Lib.trace("UNRESOLVED: "+msg);
    }
    static public function done() {
        printtotals();
 flash.Lib.trace("__END_OF_TEST__");
    }
}
