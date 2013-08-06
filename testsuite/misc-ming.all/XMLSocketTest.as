
#include "../actionscript.all/check.as"

xmlArray = [];
xmlArray[0] = 'Plain text';
xmlArray[1] = 'Plain *NEWLINE* text';
xmlArray[2] = 'Plain *NULL* text';
xmlArray[3] = 'Plain *NULL**NEWLINE* text';
xmlArray[4] = '<xml>Some XML</xml>';
xmlArray[5] = '<xml>Some XML*NEWLINE*</xml>';
xmlArray[6] = '<xml>Some XML*NULL*</xml>';
xmlArray[7] = '<xml>Some XML*NEWLINE**NULL*</xml>';
xmlArray[8] = undefined;
xmlArray[9] = 9;
xmlArray[10] = "";
a = "";
for (i = 0; i < 250; ++i) {
 a += "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"; // 60
};
xmlArray[11] = a;
xmlArray[12] = 'Last Item';
xmlArray[13] = 'closeNow';
xmlArray[14] = 'moo';

expectedArray = new Array();
expectedArray[0] = 'Plain text';
expectedArray[1] = 'Plain \n text';
expectedArray[2] = 'Plain ';
expectedArray[3] = ' text';
expectedArray[4] = 'Plain ';
expectedArray[5] = '\n text';
expectedArray[6] = '<xml>Some XML</xml>';
expectedArray[7] = '<xml>Some XML\n</xml>';
expectedArray[8] = '<xml>Some XML';
expectedArray[9] = '</xml>';
expectedArray[10] = '<xml>Some XML\n';
expectedArray[11] = '</xml>';
expectedArray[12] = 'undefined';
expectedArray[13] = 9;
expectedArray[14] = '';
// Don't check 15 (we'll only check length)
expectedArray[16] = 'Last Item';

 gc = 0;
 wait = 0;
 count = -1;
 connected = false;

 var myXML;

function handleConnect(connectionStatus) {
   check_equals(connectionStatus, true);
   if (connectionStatus) {                                      
       trace('Connected');                
       connected = true;
       if (gc < xmlArray.length) {
           myXML.send(xmlArray[gc++]);
       }
   }                                      
   else { trace('Initial connection failed!'); }  
};                                         

// Store data and send next lot.
function handleData(data) {       
    receivedArray.push(data);
    myXML.send(xmlArray[gc++]);
};                                         

function handleDisconnect() {              
    trace('Connection lost.');
    checkResults();
};                                     

function test1()
{
  trace("-- RUNNING TEST1 --");
  myXML = new XMLSocket;
  myXML.onConnect = handleConnect;
  myXML.onData = handleData;
  myXML.onClose = handleDisconnect;
  receivedArray = new Array();

  ret = myXML.connect("localhost", 2229);
  check_equals(ret, true);
}

// Check that XMLSocket can be instanciated from subclass
// See https://savannah.gnu.org/bugs/?38084
function test2()
{
  trace("-- RUNNING TEST2 --");
  receivedArray = new Array();

  SC1 = function() { };
  SC1.name = 'SC1 ctor';
  SC1.prototype = new XMLSocket();

  // None of these get called
  SC1.prototype.onConnect = function() { fail("SC1 prototype onConnect called"); };
  SC1.prototype.onData = function() { fail("SC1 prototype onData called"); };
  SC1.prototype.onClose = function() { fail("SC1 prototype onClose called"); };

  //trace("SC1.constructor is: " + SC1.constructor);
  //trace("SC1.constructor.name is: " + SC1.constructor.name);
  //trace("SC1.__constructor__ is: " + SC1.__constructor__);
  
  SC2 = function() { };
  SC2.prototype = new SC1();

  // None of these get called
  SC2.prototype.onConnect = function() { fail("SC2 prototype onConnect called"); };
  SC2.prototype.onData = function() { fail("SC2 prototype onData called"); };
  SC2.prototype.onClose = function() { fail("SC2 prototype onClose called"); };
  
  
  trace('About to instanciate an SC2 now');
  o = new SC2();
  o.onConnect = function() { fail("instance onConnect called"); };
  o.onData = function() { fail("instance onData called"); };
  o.onClose = function() { fail("instance onClose called"); };

  ret = o.connect("localhost", 2229);
  xcheck_equals(ret, true);
  nextTest();
}

currtest = 0;
tests = new Array();
tests.push(test1);
tests.push(test2);

function nextTest()
{
  if ( tests.length > currtest ) {
    tests[currtest++]();
  } else {
    trace("ENDOFTEST");
    loadMovie ("FSCommand:quit", "");
  }
}

nextTest();
stop();


function checkResults() {

    check_equals(receivedArray[0], expectedArray[0]);
    check_equals(receivedArray[1], expectedArray[1]);
    check_equals(receivedArray[2], expectedArray[2]);
    check_equals(receivedArray[3], expectedArray[3]);
    check_equals(receivedArray[4], expectedArray[4]);
    check_equals(receivedArray[5], expectedArray[5]);
    check_equals(receivedArray[6], expectedArray[6]);
    check_equals(receivedArray[7], expectedArray[7]);
    check_equals(receivedArray[8], expectedArray[8]);
    check_equals(receivedArray[9], expectedArray[9]);
    check_equals(receivedArray[10], expectedArray[10]);    
    check_equals(receivedArray[11], expectedArray[11]); 
    // NOTE: this ('undefined') fails with LNX 10,0,12,10
    check_equals(receivedArray[12], expectedArray[12]); 
    check_equals(receivedArray[13], expectedArray[13]);         
    check_equals(receivedArray[14], expectedArray[14]);
    check_equals(receivedArray[15].length, 15000);         
    check_equals(receivedArray[15].charAt(0), 'a');         
    check_equals(receivedArray[16], expectedArray[16]);
    nextTest();
};
    
