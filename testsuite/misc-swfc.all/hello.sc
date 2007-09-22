.flash bbox=200x200 filename="hello.swf" version=6 fps=30

//load dejagnu library. This must go after the .flash tag

.frame 1
  .action:
    #include "Dejagnu.sc"

    trace("Hello World!");
    var abc = 123;
    check_equals(abc, 123);  // check something is equal
    //xcheck_equals(abc, 456); // check something is equal, but expect gnash it to fail
    var qux = true;
    check(qux); // check a boolean is true
    Dejagnu.done(); // don't forget to call this, or the test will not complete
                    // Note that done() automatically calls stop();
  .end
.end
