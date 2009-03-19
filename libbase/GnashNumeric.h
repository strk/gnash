
#ifndef GNASH_NUMERIC_H
#define GNASH_NUMERIC_H

namespace gnash {

// Using a possible built-in pi constant M_PI, which is not in
// the C++ standard, has no greate advantage, so we will use this
// one. Make it as accurate as you like.
static const double PI = 3.14159265358979323846;

inline bool
isFinite(double d)
{
#if defined(HAVE_FINITE) && !defined(HAVE_ISFINITE)
    return (finite(d));
#else
    // Put using namespace std; here if you have to
    // put it anywhere.
    using namespace std;
    return (isfinite(d));
#endif
}

inline double
infinite_to_zero(double x)
{
    return isFinite(x) ? x : 0.0;
}

template <typename T>
inline T
clamp(T i, T min, T max)
{
	assert(min <= max);
	return std::max<T>(min, std::min<T>(i, max));
}

inline float
flerp(float a, float b, float f)
{
    return (b - a) * f + a;
}

inline int
frnd(float f) 
{
    return static_cast<int>(f + 0.5f);
}

inline double
twipsToPixels(int i) 
{ 
    return static_cast<double>(i / 20.0); 
}

template<size_t Factor>
boost::int32_t
truncateWithFactor(double a)
{ 

    const double factor = static_cast<double>(Factor);

    // This truncates large values without relying on undefined behaviour.
    // For very large values of 'a' it is noticeably slower than the UB
    // version (due to fmod), but should always be legal behaviour. For
    // ordinary values (within ±1.07374e+08 pixels) it is comparable to
    // the UB version for speed. Because values outside the limit are
    // extremely rare, using this safe version has no implications for
    // performance under normal circumstances.
    static const double upperUnsignedLimit =
                std::numeric_limits<boost::uint32_t>::max() + 1.0;
    static const double upperSignedLimit =
                std::numeric_limits<boost::int32_t>::max() / factor;
    static const double lowerSignedLimit =
                std::numeric_limits<boost::int32_t>::min() / factor;

    if (a >= lowerSignedLimit && a <= upperSignedLimit) {
        return static_cast<boost::int32_t>(a * factor);
    }

    // This slow truncation happens only in very unlikely cases.
    return a >= 0 ?
        static_cast<boost::uint32_t>(
                std::fmod(a * factor, upperUnsignedLimit))
        : 
        -static_cast<boost::uint32_t>(
                std::fmod(-a * factor, upperUnsignedLimit));
}

// truncate when overflow occurs.
inline boost::int32_t
pixelsToTwips(double a)
{
    return truncateWithFactor<20>(a);
}

} // namespace gnash

#endif
