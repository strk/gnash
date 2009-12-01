
#ifndef GNASH_AS_CLASS_H
#define GNASH_AS_CLASS_H

namespace gnash {
namespace abc {


/// The implementation of a 'Class' type in ActionScript 3.
//
/// A Class is a first-class type, i.e. it can be referenced itself in
/// ActionScript.
class as_class : public as_object
{
public:
    as_class() {}
    virtual ~as_class() {}

};

} // namespace abc
} // namespace gnash

#endif
