#ifndef _CHECK_AS_
#define _CHECK_AS_

#define _INFO_ ' ['+__FILE__+':'+__LINE__+']'

#define check_equals(a, b) Dejagnu.check_equals(a, b, _INFO_);
//#define check_equals(a, b, msg) Dejagnu.check(a, b, msg + _INFO_);

#define xcheck_equals(a, b) Dejagnu.xcheck_equals(a, b, _INFO_);
//#define xcheck_equals(a, b, msg) Dejagnu.check(a, b, msg + _INFO_);

#define check(a) Dejagnu.check(a, _INFO_);
#define xcheck(a) Dejagnu.xcheck(a, _INFO_);

#define pass(text) Dejagnu.pass(text + _INFO_)
#define xpass(text) Dejagnu.xpass(text + _INFO_)
#define fail(text) Dejagnu.fail(text + _INFO_)
#define xfail(text) Dejagnu.xfail(text + _INFO_)
#define pass(text) Dejagnu.pass(text + _INFO_)
#define untested(text) Dejagnu.untested(text + _INFO_)
#define unresolved(text) Dejagnu.unresolved(text + _INFO_)

#define note(text) Dejagnu.note(text + _INFO_);


#endif
