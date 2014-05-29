// BitmapData_as.cpp:  ActionScript "BitmapData" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "BitmapData_as.h"

#include <vector>
#include <sstream>
#include <algorithm>
#include <queue>
#include <boost/random.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/tuple/tuple.hpp>
#include <array>
#include <cmath>

#include "MovieClip.h"
#include "GnashImage.h"
#include "DisplayObject.h"
#include "as_object.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "GnashException.h" 
#include "VM.h" 
#include "Renderer.h"
#include "RunResources.h"
#include "namedStrings.h"
#include "Movie.h"
#include "movie_definition.h"
#include "Transform.h"
#include "ASConversions.h"
#include "flash/geom/ColorTransform_as.h"
#include "NativeFunction.h"
#include "GnashNumeric.h"
#include "Array_as.h"

namespace gnash {

namespace {

    as_value bitmapdata_applyFilter(const fn_call& fn);
    as_value bitmapdata_clone(const fn_call& fn);
    as_value bitmapdata_colorTransform(const fn_call& fn);
    as_value bitmapdata_copyChannel(const fn_call& fn);
    as_value bitmapdata_copyPixels(const fn_call& fn);
    as_value bitmapdata_dispose(const fn_call& fn);
    as_value bitmapdata_draw(const fn_call& fn);
    as_value bitmapdata_fillRect(const fn_call& fn);
    as_value bitmapdata_floodFill(const fn_call& fn);
    as_value bitmapdata_generateFilterRect(const fn_call& fn);
    as_value bitmapdata_getColorBoundsRect(const fn_call& fn);
    as_value bitmapdata_getPixel(const fn_call& fn);
    as_value bitmapdata_getPixel32(const fn_call& fn);
    as_value bitmapdata_hitTest(const fn_call& fn);
    as_value bitmapdata_merge(const fn_call& fn);
    as_value bitmapdata_noise(const fn_call& fn);
    as_value bitmapdata_paletteMap(const fn_call& fn);
    as_value bitmapdata_perlinNoise(const fn_call& fn);
    as_value bitmapdata_pixelDissolve(const fn_call& fn);
    as_value bitmapdata_scroll(const fn_call& fn);
    as_value bitmapdata_setPixel(const fn_call& fn);
    as_value bitmapdata_setPixel32(const fn_call& fn);
    as_value bitmapdata_threshold(const fn_call& fn);
    as_value bitmapdata_height(const fn_call& fn);
    as_value bitmapdata_rectangle(const fn_call& fn);
    as_value bitmapdata_transparent(const fn_call& fn);
    as_value bitmapdata_width(const fn_call& fn);
    as_value bitmapdata_loadBitmap(const fn_call& fn);
    as_value bitmapdata_compare(const fn_call& fn);
    as_value bitmapdata_ctor(const fn_call& fn);

    void attachBitmapDataInterface(as_object& o);
    void attachBitmapDataStaticProperties(as_object& o);
    as_value get_flash_display_bitmap_data_constructor(const fn_call& fn);

    /// Get an iterator to the pixel at (x, y)
    //
    /// Please note: each invocation of this function must check for and
    /// retrieve the stored bitmap data, so if you need more than one pixel,
    /// call this once and work out how much you need to move the iterator.
    BitmapData_as::iterator pixelAt(const BitmapData_as& bd, size_t x,
            size_t y);

    /// Set the pixel at (x, y) to the specified ARGB colour.
    //
    /// Note: calls pixelAt(), so do not use this more than necessary!
    void setPixel32(const BitmapData_as& bd, size_t x, size_t y,
            std::uint32_t color);

    /// Set the RGB values at (x, y) to the RGB components of a colour.
    //
    /// Alpha values of the target pixel are left unchanged.
    //
    /// Note: calls pixelAt(), so do not use this more than necessary!
    void setPixel(const BitmapData_as& bd, size_t x, size_t y,
            std::uint32_t color);

    std::uint32_t getPixel(const BitmapData_as& bd, size_t x, size_t y);

    /// Get the overlapping part of a rectangle and a Bitmap
    //
    /// The values are adjusted so that the rectangle is wholly inside the
    /// bitmap. If no part of the rectangle overlaps, either w or h
    /// will be 0.
    //
    /// @param x    The x co-ordinate of the top left corner.
    /// @param y    The y co-ordinate of the top left corner.
    /// @param w    The width of the rectangle.
    /// @param h    The height of the rectangle.
    void adjustRect(int& x, int& y, int& w, int& h, const BitmapData_as& b);

    std::uint32_t setChannel(std::uint32_t targ, std::uint8_t bitmask,
            std::uint8_t value);

    std::uint8_t getChannel(std::uint32_t src, std::uint8_t bitmask);

    void floodFill(const BitmapData_as& bd, size_t startx, size_t starty,
            std::uint32_t old, std::uint32_t fill);

    /// Fill a rectangle of the BitmapData_as
    //
    /// Do not call on a disposed BitmapData_as!
    //
    /// @param bd       The BitmapData_as to operate on.
    /// @param x        The x co-ordinate of the rectangle's top left corner.
    /// @param y        The y co-ordinate of the rectangle's top left corner.
    /// @param w        The width of the rectangle.
    /// @param h        The height of the rectangle.
    /// @param color    The ARGB colour to fill with.
    void fillRect(const BitmapData_as& bd, int x, int y, int w, int h,
            std::uint32_t color);

    inline bool oneBitSet(std::uint8_t mask) {
        return mask == (mask & -mask);
    }
}

/// Local functors.
namespace {

/// Random number generator for noise
//
/// This uses the fastest RNG available; it's still more
/// homogeneous than the Adobe one.
template<typename RNG = boost::rand48>
struct Noise
{
    typedef RNG Rand;
    typedef boost::uniform_int<> Dist;
    typedef boost::variate_generator<Rand, boost::uniform_int<> > Gen;

    /// Create a PRNG to supply uniformly distributed numbers.
    //
    /// @param seed     A seed for the pseudo random numbers.
    /// @param low      The lowest value in the uniform range.
    /// @param high     The highest value in the uniform range.
    Noise(int seed, int low, int high)
        :
        rng(seed),
        dist(low, high),
        uni(rng, dist)
    {}

    /// Get a random int between in the range specified at construction.
    int operator()() {
        return uni();
    }

    /// Get a random int between 0 and val.
    //
    /// This is for use by std::random_shuffle().
    int operator()(int val) {
        // Note: in versions of boost newer than 1.35 or so, we can just call
        // uni(val), but we still aim to support 1.35 (and 1.34 if possible).
        typedef boost::random_number_generator<Gen> Adapter;
        return Adapter(uni)(val);
    }

private:
    Rand rng;
    Dist dist;
    Gen uni;
};

template<typename NoiseGenerator>
struct NoiseAdapter
{
    NoiseAdapter(NoiseGenerator& n, std::uint8_t bitmask, bool grey)
        :
        _gen(n),
        _bitmask(bitmask),
        _greyscale(grey)
    {}

    /// Generate a 32-bit ARGB noise value.
    std::uint32_t operator()() {

        if (_greyscale) {
            std::uint8_t val = _gen();
            return 0xff000000 | val | val << 8 | val << 16;
        }

        std::uint32_t ret = 0xff000000;

        if (_bitmask & BitmapData_as::CHANNEL_RED) {
            ret |= (_gen() << 16);
        }
        if (_bitmask & BitmapData_as::CHANNEL_GREEN) {
            ret |= _gen() << 8;
        }
        if (_bitmask & BitmapData_as::CHANNEL_BLUE) {
            ret |= _gen();
        }
        if (_bitmask & BitmapData_as::CHANNEL_ALPHA) {
            // Alpha is 0xff by default.
            const std::uint8_t rd = _gen();
            ret &= (~rd) << 24;
        }
        return ret;
    }

private:
    NoiseGenerator& _gen;
    const std::uint8_t _bitmask;
    const bool _greyscale;
};

/// Copy a single channel from one ARGB value to a channel in another.
//
/// Due to peculiarities in the Adobe implementation, this also supports
/// setting them to black.
template<typename Iterator>
struct CopyChannel
{
    typedef boost::tuple<Iterator, Iterator> iterator_tuple;
    typedef boost::zip_iterator<iterator_tuple> iterator_type;

    CopyChannel(bool multiple, std::uint8_t srcchans,
            std::uint8_t destchans)
        :
        _multiple(multiple),
        _srcchans(srcchans),
        _destchans(destchans)
    {}

    std::uint32_t operator()(typename iterator_type::value_type p) const {
        // If multiple source channels, we set the destination channel
        // to black. Else to the value of the requested channel.
        const std::uint8_t val = _multiple ?
            0 : getChannel(boost::get<0>(p), _srcchans);
        return setChannel(boost::get<1>(p), _destchans, val);
    }

private:
    const bool _multiple;
    const std::uint8_t _srcchans;
    const std::uint8_t _destchans;

};

template<typename T> 
T easeCurve(T t)
{
    return t * t * (3.0 - 2.0 * t);
}

template<typename T>
void normalize(T& a, T& b)
{
    const T s = std::sqrt(a * a + b * b);
    a /= s;
    b /= s;
}

/// Generate Perlin noise
//
/// @tparam T       A floating point type for the generated values.
/// @tparam B       The size of the permutation table.
/// @tparam Offset  An offset for generating non-identical patterns with the
///                 same Perlin noise generator.
template<typename T, std::uint32_t Size = 0x100,
    std::uint32_t Offset = 1327>
struct PerlinNoise
{
    typedef T value_type;

    /// Create a Perlin noise generator with a random seed.
    //
    /// @param seed     A seed for the PRNG. Given the same seed the
    ///                 generator will create the same pseudo-random pattern.
    PerlinNoise(int seed)
    {
        init(seed);
    }

    std::uint32_t size() const {
        return Size;
    }

    /// Get a noise value for the co-ordinates x and y.
    T operator()(T x, T y, const size_t step = 0) {

        // Point to the left
        size_t bx0;
        // Point to the right
        size_t bx1;
        // Point above
        size_t by0;
        // Point below.
        size_t by1;

        // Actual distance from the respective integer positions.
        T rx0, rx1, ry0, ry1;

        // Compute integer positions of surrounding points and
        // vectors.
        setup(x, bx0, bx1, rx0, rx1, step);
        setup(y, by0, by1, ry0, ry1, step);

        assert(bx0 < permTable.size());
        assert(bx1 < permTable.size());

        const int i = permTable[bx0];
        const int j = permTable[bx1];

        assert(i + by0 < permTable.size());
        assert(j + by0 < permTable.size());
        assert(i + by1 < permTable.size());
        assert(j + by0 < permTable.size());

        // Permute values to get indices in the lookup tables.
        const size_t b00 = permTable[i + by0];
        const size_t b10 = permTable[j + by0];
        const size_t b01 = permTable[i + by1];
        const size_t b11 = permTable[j + by1];

        // Dot product of vectors and gradients.
        const T u = rx0 * g2[b00][0] + ry0 * g2[b00][1];
        const T v = rx1 * g2[b10][0] + ry0 * g2[b10][1];
        const T u1 = rx0 * g2[b01][0] + ry1 * g2[b01][1];
        const T v1 = rx1 * g2[b11][0] + ry1 * g2[b11][1];

        // Cubic ease curve for interpolation.
        const T sx = easeCurve(rx0);
        const T sy = easeCurve(ry0);

        // X-axis interpolation.
        const T a = lerp<T>(u, v, sx);
        const T b = lerp<T>(u1, v1, sx);

        // Y-axis interpolation.
        return lerp<T>(a, b, sy);
    }

private:

    static void setup(T i, size_t& b0, size_t& b1, T& r0, T& r1, size_t step) {

        const T t = i + Offset * step;

        // Let the compiler optimize this if Size is a power of two.
        b0 = (static_cast<size_t>(t)) % Size;
        b1 = (b0 + 1) % Size;
        
        // Calculate vectors to surrounding points.
        r0 = t - static_cast<size_t>(t);
        r1 = r0 - 1.0;
    }

    void init(int seed) {

        Noise<> noise(seed, 0, RAND_MAX);

        for (size_t i = 0 ; i < Size; ++i) {
            permTable[i] = i;
            for (size_t j = 0; j < 2; ++j) {
                // If Size is an unsigned int this expression:
                // noise() * 2 * Size - Size 
                // is unsigned (but if it's an unsigned short, the result is
                // signed!) so convert here in case of any future changes.
                const int b = Size;
                g2[i][j] = static_cast<T>(noise() % (2 * b) - b) / b;
            }
            normalize(g2[i][0], g2[i][1]);
        }

        std::random_shuffle(permTable.begin(), permTable.begin() + Size, noise);

        std::copy(
            boost::make_zip_iterator(
                boost::make_tuple(permTable.begin(), g2.begin())),
            boost::make_zip_iterator(
                boost::make_tuple(permTable.begin(), g2.begin())) + Size,
            boost::make_zip_iterator(
                boost::make_tuple(permTable.begin(), g2.begin())) + Size);

        std::copy(
            boost::make_zip_iterator(
                boost::make_tuple(permTable.begin(), g2.begin())),
            boost::make_zip_iterator(
                boost::make_tuple(permTable.begin(), g2.begin())) + 2,
            boost::make_zip_iterator(
                boost::make_tuple(permTable.begin(), g2.begin())) + 2 * Size);
    }

    // A random permutation table.
    std::array<size_t, Size * 2 + 2> permTable;

    // The gradient stuff.
    std::array<std::array<T, 2>, Size * 2 + 2> g2;
};

/// Store offsets.
struct Vector
{
    Vector(std::int32_t x, std::int32_t y) : x(x), y(y) {}
    std::int32_t x;
    std::int32_t y;
};

/// Transform negative offsets into positive ones
//
/// The PerlinNoise generator only handles positive co-ordinates, so
/// transform negative ones into an offset from the end of the grid.
//
/// We have to take base into account (I think), but not octave because
/// octaves are all exact factors of the base.
struct
VectorTransformer
{
    /// Construct a VectorTransformer for an x by y grid.
    //
    /// @param x    The product of the grid size and the x base.
    /// @param y    The product of the grid size and the y base.
    VectorTransformer(size_t x, size_t y) : _x(x), _y(y) {}

    /// Get a point within the grid.
    //
    /// Positive co-ordinates are left unchanged, negative ones translated
    /// to an offset from the end of the grid.
    Vector operator()(Vector const& p) const {
        if (p.x >= 0 && p.y >= 0) return p;
        const int x = p.x > 0 ? p.x : _x - std::abs(p.x) % _x;
        const int y = p.y > 0 ? p.y : _y - std::abs(p.y) % _y;
        return Vector(x, y);
    }
private:
    const size_t _x;
    const size_t _y;
};

/// Adapt the PerlinNoise generator for ActionScript's needs.
//
/// @tparam Generator   A type with a double operator()(size_t, size_t, size_t)
///                     that returns a perlin noise value when passed two
///                     positive co-ordinates and a sequence offset value.
template<typename Generator>
struct PerlinAdapter
{
    /// Create an adapter for Perlin noise.
    //
    /// @param g        The PerlinNoise generator.
    /// @param octaves  The number of octaves to generate
    /// @param baseX    The scale of the grid along the X axis.
    /// @param baseY    The scale of the grid along the Y axis.
    /// @param fractal  Whether to apply |f(n)| (false) or f(n) (true) to
    ///                 the colour values.
    /// @param offsets  A vector of offsets; each element applies to a
    ///                 successive octave. Any remaining octaves will have no
    ///                 offset applied.
    PerlinAdapter(Generator& g, size_t octaves, double baseX, double baseY,
            bool fractal, const std::vector<Vector>& offsets)
        :
        _gen(g),
        _octaves(octaves),
        _baseX(baseX),
        _baseY(baseY),
        _fractal(fractal)
    {
        // Make sure all offsets represent a valid positive value within
        // the grid.
        std::transform(offsets.begin(), offsets.end(), 
                std::back_inserter(_offsets),
                VectorTransformer(_baseX * _gen.size(), _baseY * _gen.size()));
    }

    /// Return a noise value for the co-ordinate (x, y).
    //
    /// Optionally you can pass a sequence offset so that the noise pattern
    /// is overlaid at an offset; this means that the same generator can be
    /// used for several channels without them appearing to be the same.
    //
    /// Fractal noise adds f(i * freq) * amp to the mid value.
    /// Normal noise adds |f(i * freq) * amp| to 0.
    typename Generator::value_type operator()(size_t x, size_t y,
            size_t step = 0) {
        
        // Starting amplitude
        size_t amp = _fractal ? 0x80 : 0xff;
        // Base x frequency.
        double xphase = _baseX;
        // Base y frequency.
        double yphase = _baseY;
        // Return value.
        double ret = _fractal ? 0x80 : 0;

        for (size_t i = 0; i < _octaves; ++i) {
   
            const Vector offset = i < _offsets.size() ? _offsets[i] :
                Vector(0, 0);

            const double n = _gen((x + offset.x) / xphase,
                    (y + offset.y) / yphase, step);

            ret += amp * (_fractal ? n : std::abs(n));

            // Halve amplitude
            amp >>= 1;
            if (!amp) break;

            // Double frequency
            xphase /= 2;
            yphase /= 2;
        }
        return ret;
    }

private:
    Generator& _gen;
    const size_t _octaves;
    const double _baseX;
    const double _baseY;
    const bool _fractal;
    std::vector<Vector> _offsets;
};

/// Index iterators by x and y position
//
/// This is a helper for floodFill to avoid using the expensive
/// pixelAt() many times.
struct PixelIndexer
{
    PixelIndexer(size_t xpos, size_t ypos, BitmapData_as::iterator p)
        :
        x(xpos),
        y(ypos),
        pix(p)
    {}
    size_t x;
    size_t y;
    BitmapData_as::iterator pix;
};

/// Convert an array to a vector of offsets.
//
/// TODO: check what really happens when one is invalid.
struct VectorPusher
{
    VectorPusher(std::vector<Vector>& offsets, VM& vm)
        :
        _offsets(offsets),
        _vm(vm)
    {}

    void operator()(const as_value& val) {
        as_object* p = toObject(val, _vm);
        if (!p) return;

        as_value x, y;
        if (!p->get_member(NSV::PROP_X, &x)) return;
        if (!p->get_member(NSV::PROP_Y, &y)) return;
        _offsets.emplace_back(toInt(x, _vm), toInt(y, _vm));
    }

private:
    std::vector<Vector>& _offsets;
    VM& _vm;
};

} // anonymous namespace

BitmapData_as::BitmapData_as(as_object* owner,
        std::unique_ptr<image::GnashImage> im)
    :
    _owner(owner),
    _cachedBitmap(nullptr)
{
    assert(im->width() <= 2880);
    assert(im->height() <= 2880);
    
    // If there is a renderer, cache the image there, otherwise we store it.
    Renderer* r = getRunResources(*_owner).renderer();
    if (r) _cachedBitmap = r->createCachedBitmap(std::move(im));
    else _image.reset(im.release());
}
    
void
BitmapData_as::setReachable() 
{
    std::for_each(_attachedObjects.begin(), _attachedObjects.end(),
            std::mem_fun(&DisplayObject::setReachable));
    _owner->setReachable();
}

void
BitmapData_as::updateObjects() const
{
    std::for_each(_attachedObjects.begin(), _attachedObjects.end(),
            std::mem_fun(&DisplayObject::update));
}

void
BitmapData_as::dispose()
{
    if (_cachedBitmap) _cachedBitmap->dispose();
    _cachedBitmap = nullptr;
    _image.reset();
    updateObjects();
}

void
BitmapData_as::draw(MovieClip& mc, const Transform& transform)
{
    if (disposed()) return;

    image::GnashImage& im = *data();

    Renderer* base = getRunResources(*_owner).renderer();
    if (!base) {
        log_debug("BitmapData.draw() called without an active renderer");
        return;
    }

    Renderer::Internal in(*base, im);
    Renderer* internal = in.renderer();
    if (!internal) {
        log_debug("Current renderer does not support internal rendering");
        return;
    }

    mc.draw(*internal, transform);
    updateObjects();
}

// extern 
void
bitmapdata_class_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(uri,
            get_flash_display_bitmap_data_constructor, flags);
}

void
registerBitmapDataNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(bitmapdata_getPixel, 1100, 1);
    vm.registerNative(bitmapdata_setPixel, 1100, 2);
    vm.registerNative(bitmapdata_fillRect, 1100, 3);
    vm.registerNative(bitmapdata_copyPixels, 1100, 4);
    vm.registerNative(bitmapdata_applyFilter, 1100, 5);
    vm.registerNative(bitmapdata_scroll, 1100, 6);
    vm.registerNative(bitmapdata_threshold, 1100, 7);
    vm.registerNative(bitmapdata_draw, 1100, 8);
    vm.registerNative(bitmapdata_pixelDissolve, 1100, 9);
    vm.registerNative(bitmapdata_getPixel32, 1100, 10);
    vm.registerNative(bitmapdata_setPixel32, 1100, 11);
    vm.registerNative(bitmapdata_floodFill, 1100, 12);
    vm.registerNative(bitmapdata_getColorBoundsRect, 1100, 13);
    vm.registerNative(bitmapdata_perlinNoise, 1100, 14);
    vm.registerNative(bitmapdata_colorTransform, 1100, 15);
    vm.registerNative(bitmapdata_hitTest, 1100, 16);
    vm.registerNative(bitmapdata_paletteMap, 1100, 17);
    vm.registerNative(bitmapdata_merge, 1100, 18);
    vm.registerNative(bitmapdata_noise, 1100, 19);
    vm.registerNative(bitmapdata_copyChannel, 1100, 20);
    vm.registerNative(bitmapdata_clone, 1100, 21);
    vm.registerNative(bitmapdata_dispose, 1100, 22);
    vm.registerNative(bitmapdata_generateFilterRect, 1100, 23);
    vm.registerNative(bitmapdata_compare, 1100, 24);
    
    vm.registerNative(bitmapdata_width, 1100, 100);
    vm.registerNative(bitmapdata_height, 1100, 101);
    vm.registerNative(bitmapdata_rectangle, 1100, 102);
    vm.registerNative(bitmapdata_transparent, 1100, 103);
    
    vm.registerNative(bitmapdata_loadBitmap, 1100, 40);

}

namespace {

as_value
bitmapdata_applyFilter(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    UNUSED(ptr);
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
bitmapdata_clone(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    BitmapData_as* bm = ensure<ThisIsNative<BitmapData_as> >(fn);
    if (bm->disposed()) return as_value();

    const size_t width = bm->width();
    const size_t height = bm->height();

    std::unique_ptr<image::GnashImage> im;
    if (bm->transparent()) {
        im.reset(new image::ImageRGBA(width, height));
    }
    else {
        im.reset(new image::ImageRGB(width, height));
    }
    // Note that it would be much faster to copy the pixels, but BitmapData
    // currently doesn't expose a way to do this.
    std::copy(bm->begin(), bm->end(), image::begin<image::ARGB>(*im));

    Global_as& gl = getGlobal(fn);
    as_object* ret = createObject(gl);
    const as_value& proto = getMember(*obj, NSV::PROP_uuPROTOuu);
    if (proto.is_object()) {
        ret->set_member(NSV::PROP_uuPROTOuu, proto);
    }

    ret->setRelay(new BitmapData_as(ret, std::move(im)));

    return as_value(ret);
}

as_value
bitmapdata_colorTransform(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    UNUSED(ptr);
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
bitmapdata_copyChannel(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (ptr->disposed()) return as_value();

    if (fn.nargs < 5) {
        // log error
        return as_value();
    }

    as_object* o = toObject(fn.arg(0), getVM(fn));
    BitmapData_as* source;
    if (!isNativeType(o, source) || source->disposed()) {
        // First argument is not a BitmapData or is disposed.
        return as_value();
    }

    as_object* rect = toObject(fn.arg(1), getVM(fn));
    if (!rect) {
        // Second argument is not an object
        return as_value();
    }

    as_value x, y, w, h;
    
    rect->get_member(NSV::PROP_X, &x);
    rect->get_member(NSV::PROP_Y, &y);
    rect->get_member(NSV::PROP_WIDTH, &w);
    rect->get_member(NSV::PROP_HEIGHT, &h);    
    
    as_object* destpoint = toObject(fn.arg(2), getVM(fn));
    as_value px, py;
    
    destpoint->get_member(NSV::PROP_X, &px);
    destpoint->get_member(NSV::PROP_Y, &py);

    // TODO: check what should happen if the argument overflows or
    // is negative (currently it is truncated and made positive.

    // The source channel mask
    const std::uint8_t srcchans =
        std::abs(toInt(fn.arg(3), getVM(fn))) & 15;

    // The destination channel mask
    const std::uint8_t destchans =
        std::abs(toInt(fn.arg(4), getVM(fn))) & 15;

    // If more than one destination channel is specified,
    // nothing happens.
    if (!oneBitSet(destchans)) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("BitmapData.copyChannel(). Multiple "
                          "destination channels are not supported"));
        );
        return as_value();
    }

    const bool multiple = !oneBitSet(srcchans);

    // Find true source rect and true dest rect.
    int sourceX = toInt(x, getVM(fn));
    int sourceY = toInt(y, getVM(fn));
    int sourceW = toInt(w, getVM(fn));
    int sourceH = toInt(h, getVM(fn));

    int destX = toInt(px, getVM(fn));
    int destY = toInt(py, getVM(fn));

    // Any part of the source rect that is not in the image (i.e.
    // above or left) is concatenated to the destination offset.
    if (sourceX < 0) destX -= sourceX;
    if (sourceY < 0) destY -= sourceY;

    adjustRect(sourceX, sourceY, sourceW, sourceH, *source);
    if (sourceW == 0 || sourceH == 0) {
        // The source rect does not overlap with source bitmap
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("BitmapData.copyChannel(): no part of source rectangle"
                          "overlaps with the source BitmapData"));
        );
        return as_value();
    }

    // The dest width starts the same as the adjusted source width.
    int destW = sourceW;
    int destH = sourceH;

    adjustRect(destX, destY, destW, destH, *ptr);
    if (destW == 0 || destH == 0) {
        // The target rect does not overlap with source bitmap
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("BitmapData.copyChannel(): destination area is "
                          "wholly outside the destination BitmapData"));
        );
        return as_value();
    }

    BitmapData_as::iterator targ = pixelAt(*ptr, destX, destY);
    BitmapData_as::iterator src = pixelAt(*source, sourceX, sourceY);

    // Just being careful...
    assert(sourceX + destW <= static_cast<int>(source->width()));
    assert(sourceY + destH <= static_cast<int>(source->height()));
    assert(destX + destW <= static_cast<int>(ptr->width()));
    assert(destY + destH <= static_cast<int>(ptr->height()));

    // Copy for the width and height of the *dest* image.
    // We have already ensured that the copied area
    // is inside both bitmapdatas
    typedef CopyChannel<BitmapData_as::iterator> Copier;
    Copier c(multiple, srcchans, destchans);

    const size_t ourwidth = ptr->width();
    const size_t srcwidth = source->width();

    // Note that copying the same channel to a range starting in the
    // source range produces unexpected effects because the source
    // range is changed while it is being copied. This is verified
    // to happen with the Adobe player too.
    for (int i = 0; i < destH; ++i) {
        Copier::iterator_type zip(boost::make_tuple(src, targ));
        std::transform(zip, zip + destW, targ, c);
        targ += ourwidth;
        src += srcwidth;
    }

    ptr->updateObjects();

    return as_value();
}

// sourceBitmap: BitmapData,
// sourceRect: Rectangle,
// destPoint: Point,
// [alphaBitmap: BitmapData],
// [alphaPoint: Point],
// [mergeAlpha: Boolean]
as_value
bitmapdata_copyPixels(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (ptr->disposed()) return as_value();

    if (fn.nargs < 3) {
        // Require three arguments? (TODO: check).
        return as_value();
    }
    if (fn.nargs > 3) {
        LOG_ONCE(log_unimpl(_("BitmapData.copyPixels(): arguments after "
                              "the first three are discarded")));
    }

    as_object* o = toObject(fn.arg(0), getVM(fn));
    BitmapData_as* source;
    if (!isNativeType(o, source) || source->disposed()) {
        // First argument is not a BitmapData or is disposed.
        return as_value();
    }

    as_object* rect = toObject(fn.arg(1), getVM(fn));
    if (!rect) {
        // Second argument is not an object
        return as_value();
    }

    as_value x, y, w, h;
    
    rect->get_member(NSV::PROP_X, &x);
    rect->get_member(NSV::PROP_Y, &y);
    rect->get_member(NSV::PROP_WIDTH, &w);
    rect->get_member(NSV::PROP_HEIGHT, &h);

    int destX = 0;
    int destY = 0;
    
    // Find true source rect and true dest rect.
    as_object* destpoint = toObject(fn.arg(2), getVM(fn));
    if (destpoint) {
        as_value px, py;
    
        destpoint->get_member(NSV::PROP_X, &px);
        destpoint->get_member(NSV::PROP_Y, &py);

        destX = toInt(px, getVM(fn));
        destY = toInt(py, getVM(fn));
    }

    // TODO: remaining arguments.

    int sourceX = toInt(x, getVM(fn));
    int sourceY = toInt(y, getVM(fn));
    int sourceW = toInt(w, getVM(fn));
    int sourceH = toInt(h, getVM(fn));

    // Any part of the source rect that is not in the image (i.e.
    // above or left) is concatenated to the destination offset.
    if (sourceX < 0) destX -= sourceX;
    if (sourceY < 0) destY -= sourceY;

    adjustRect(sourceX, sourceY, sourceW, sourceH, *source);
    if (sourceW == 0 || sourceH == 0) {
        // The source rect does not overlap with source bitmap
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("BitmapData.copyPixels(): no part of source rectangle"
                          "overlaps with the source BitmapData"));
        );
        return as_value();
    }

    // The dest width starts the same as the adjusted source width.
    int destW = sourceW;
    int destH = sourceH;

    adjustRect(destX, destY, destW, destH, *ptr);
    if (destW == 0 || destH == 0) {
        // The target rect does not overlap with source bitmap
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("BitmapData.copyPixels(): destination area is "
                          "wholly outside the destination BitmapData"));
        );
        return as_value();
    }

    BitmapData_as::iterator targ = pixelAt(*ptr, destX, destY);
    BitmapData_as::iterator src = pixelAt(*source, sourceX, sourceY);

    const bool sameImage = (ptr == source);
    const bool copyToXRange = sameImage && 
        (destX >= sourceX && destX < sourceX + destW);
    const bool copyToYRange = sameImage &&
        (destY >= sourceY && destY < sourceY + destH);

    // Just being careful...
    assert(sourceX + destW <= static_cast<int>(source->width()));
    assert(sourceY + destH <= static_cast<int>(source->height()));
    assert(destX + destW <= static_cast<int>(ptr->width()));
    assert(destY + destH <= static_cast<int>(ptr->height()));

    const size_t ourwidth = ptr->width();
    const size_t srcwidth = source->width();

    // Copy for the width and height of the *dest* image.
    // We have already ensured that the copied area
    // is inside both bitmapdatas.
    //
    // If the destination y-range starts within the source y-range, copy from
    // bottom to top.
    // If the destination x-range starts within the source x-range, copy from
    // right to left.
    if (copyToYRange) {
        assert(destH > 0);
        targ += (destH - 1) * ourwidth;
        src += (destH - 1) * srcwidth;
        // Copy from bottom to top.
        for (int i = destH; i > 0; --i) {
            if (copyToXRange) {
                std::copy_backward(src, src + destW, targ + destW);
            }
            else {
                std::copy(src, src + destW, targ);
            }
            targ -= ourwidth;
            src -= srcwidth;
        }
    }
    else {
        // Normal copy from top to bottom.
        for (int i = 0; i < destH; ++i) {
            if (copyToXRange) {
                std::copy_backward(src, src + destW, targ + destW);
            }
            else {
                std::copy(src, src + destW, targ);
            }
            targ += ourwidth;
            src += srcwidth;
        }
    }

    ptr->updateObjects();

    return as_value();
}

as_value
bitmapdata_dispose(const fn_call& fn)
{
    // Should free the memory storing the bitmap.
    // All properties afterwards are -1 (even the rectangle)
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    ptr->dispose();
    return as_value();
}

as_value
bitmapdata_draw(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("BitmapData.draw(%s) requires at least one argument"),
                ss.str());
        );
        return as_value();
    }

    as_object* o = toObject(fn.arg(0), getVM(fn));
    MovieClip* mc = get<MovieClip>(o);
    if (!mc) {

        BitmapData_as* bitmap;
        if (isNativeType(o, bitmap)) {
            LOG_ONCE(log_unimpl(_("BitmapData.draw() with BitmapData argument")));
            return as_value();
        }

        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("BitmapData.draw(%s): first argument must be a "
                          "MovieClip"), ss.str());
        );
        return as_value();
    }

    Transform t;
    if (fn.nargs > 1) {
        as_object* o = toObject(fn.arg(1), getVM(fn));
        if (o) t.matrix = toSWFMatrix(*o);
    }
    if (fn.nargs > 2) {
        as_object* o = toObject(fn.arg(2), getVM(fn));
        ColorTransform_as* tr;
        if (isNativeType(o, tr)) {
            t.colorTransform = toCxForm(*tr);
        }
    }

    SWFMatrix oldM = mc->transform().matrix;
    mc->setMatrix(t.matrix, true);
    ptr->draw(*mc, t);
    mc->setMatrix(oldM, true);

    return as_value();
}

as_value
bitmapdata_fillRect(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (fn.nargs < 2 || ptr->disposed()) return as_value();
    
    const as_value& arg = fn.arg(0);
    
    if (!arg.is_object()) {
        /// Isn't an object...
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("BitmapData.fillRect(%s): needs an object"), ss.str());
        );
        return as_value();
    }

    // This can be any object with the right properties.   
    as_object* obj = toObject(arg, getVM(fn));
    assert(obj);
    
    as_value x, y, w, h;
    
    obj->get_member(NSV::PROP_X, &x);
    obj->get_member(NSV::PROP_Y, &y);
    obj->get_member(NSV::PROP_WIDTH, &w);
    obj->get_member(NSV::PROP_HEIGHT, &h);    

    const std::uint32_t color = toInt(fn.arg(1), getVM(fn));
       
    fillRect(*ptr, toInt(x, getVM(fn)), toInt(y, getVM(fn)),
            toInt(w, getVM(fn)), toInt(h, getVM(fn)), color);
    
    return as_value();
}


// Fills the bitmap with a colour starting at point x, y.
as_value
bitmapdata_floodFill(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    
    if (fn.nargs < 3) {
        return as_value();
    }

    if (ptr->disposed()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("floodFill called on disposed BitmapData!"));
        );
        return as_value();
    }
    
    const int x = toInt(fn.arg(0), getVM(fn));
    const int y = toInt(fn.arg(1), getVM(fn));

    if (x < 0 || y < 0) {
        return as_value();
    }

    const std::uint32_t fill = toInt(fn.arg(2), getVM(fn));
    const std::uint32_t old = *pixelAt(*ptr, x, y);

    // This checks whether the colours are the same.
    floodFill(*ptr, x, y, old, fill);
    
    return as_value();
}

as_value
bitmapdata_generateFilterRect(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    UNUSED(ptr);
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
bitmapdata_getColorBoundsRect(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    UNUSED(ptr);
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
bitmapdata_getPixel(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (fn.nargs < 2) {
        return as_value();
    }

    if (ptr->disposed()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("getPixel called on disposed BitmapData!");
        );
        return as_value();
    }
    
    const int x = toInt(fn.arg(0), getVM(fn));
    const int y = toInt(fn.arg(1), getVM(fn));

    // Note: x and y are converted to size_t, so negative values becomre
    // very large and are also outside the image. This is perfectly legal!
    return static_cast<std::int32_t>(getPixel(*ptr, x, y) & 0xffffff);
}

as_value
bitmapdata_getPixel32(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (fn.nargs < 2) {
        return as_value();
    }

    if (ptr->disposed()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("getPixel32 called on disposed BitmapData!"));
        );
        return as_value();
    }
    
    const int x = toInt(fn.arg(0), getVM(fn));
    const int y = toInt(fn.arg(1), getVM(fn));
    
    // Note: x and y are converted to size_t, so negative values becomre
    // very large and are also outside the image. This is perfectly legal!
    return static_cast<std::int32_t>(getPixel(*ptr, x, y));
}


as_value
bitmapdata_hitTest(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    UNUSED(ptr);
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
bitmapdata_merge(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    UNUSED(ptr);
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
bitmapdata_noise(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (ptr->disposed()) return as_value();

    if (fn.nargs < 1) {
        return as_value();
    }
    const int seed = toInt(fn.arg(0), getVM(fn));

    const std::uint8_t low = fn.nargs > 1 ?
        clamp(toInt(fn.arg(1), getVM(fn)), 0, 255) : 0;

    const std::uint8_t high = fn.nargs > 2 ?
        clamp<int>(toInt(fn.arg(2), getVM(fn)), low, 255) : 255;

    const std::uint8_t chans = fn.nargs > 3 ?
        std::abs(toInt(fn.arg(3), getVM(fn))) & 15 :
            BitmapData_as::CHANNEL_RED |
            BitmapData_as::CHANNEL_GREEN |
            BitmapData_as::CHANNEL_BLUE;

    const bool greyscale = fn.nargs > 4 ?
        toBool(fn.arg(4), getVM(fn)) : false;

    Noise<> noise(seed, low, high);

    NoiseAdapter<Noise<> > n(noise, chans, greyscale);

    std::generate(ptr->begin(), ptr->end(), n);
    
    ptr->updateObjects();

    return as_value();
}

as_value
bitmapdata_paletteMap(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    UNUSED(ptr);
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}


as_value
bitmapdata_perlinNoise(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (ptr->disposed()) return as_value();

    if (fn.nargs < 4) {
        return as_value();
    }
    const double baseX = toNumber(fn.arg(0), getVM(fn));
    const double baseY = toNumber(fn.arg(1), getVM(fn));
    const int octave = std::max(0, toInt(fn.arg(2), getVM(fn)));

    /// Random seed.
    const int seed = toInt(fn.arg(3), getVM(fn));

    // Whether to make a tileable pattern.
    const bool stitch = fn.nargs > 4 ?
        toBool(fn.arg(4), getVM(fn)) : false;

    // If true makes a fractal noise, otherwise turbulence.
    const bool fractalNoise = fn.nargs > 5 ?
        toBool(fn.arg(5), getVM(fn)) : false;

    // Which channels to use.
    const std::uint8_t channels = fn.nargs > 6 ?
        clamp<int>(toInt(fn.arg(6), getVM(fn)), 0, 255) :
            BitmapData_as::CHANNEL_RED |
            BitmapData_as::CHANNEL_GREEN |
            BitmapData_as::CHANNEL_BLUE;

    // All channels the same
    const bool greyscale = fn.nargs > 7 ?
        toBool(fn.arg(7), getVM(fn)) : false;

    /// Collect offsets.
    //
    /// We don't currently know what happens when one of the offsets doesn't
    /// have an x or a y member, or isn't an object etc.
    std::vector<Vector> offsets;
    if (fn.nargs > 8) {
        as_object* obj = toObject(fn.arg(8), getVM(fn));
        if (obj) {
            VectorPusher pp(offsets, getVM(fn));
            foreachArray(*obj, pp);
        }
    }

    if (stitch) {
        LOG_ONCE(log_unimpl(_("BitmapData.perlinNoise() stitch value")));
    }

    if (!octave || (!channels && !greyscale)) {
        // Clear the image and return.
        std::fill(ptr->begin(), ptr->end(), 0xff000000);
        return as_value();
    }

    typedef PerlinNoise<double, 256> Generator;
    Generator gen(seed);
    PerlinAdapter<Generator> pa(gen, octave, baseX, baseY, fractalNoise,
            offsets);

    const size_t width = ptr->width();
    const bool transparent = ptr->transparent();

    size_t pixel = 0;
    for (BitmapData_as::iterator it = ptr->begin(), e = ptr->end(); it != e;
            ++it, ++pixel) {

        const size_t x = pixel % width;
        const size_t y = pixel / width;

        std::uint8_t rv = 0;

        if (greyscale || channels & BitmapData_as::CHANNEL_RED) {
            // Create one noise channel.
            const double r = pa(x, y);
            rv = clamp(r, 0.0, 255.0);
        }

        std::uint8_t av = 0xff;

        // It's just a waste of time if the BitmapData has no alpha.
        if (transparent && channels & BitmapData_as::CHANNEL_ALPHA) {
            const double a = pa(x, y, 3);
            av -= clamp(a, 0.0, 255.0);
        }

        if (greyscale) {
            // Greyscale affects all colour channels equally. If alpha the
            // alpha channel is requested, that's done seperately; otherwise
            // it's full.
            *it = (rv | rv << 8 | rv << 16 | av << 24);
            continue;
        }

        // Otherwise create data for the other channels too by using the
        // PerlinNoise object's pattern offset (this is cheaper than using a
        // separate generator)
        std::uint8_t gv = 0;
        std::uint8_t bv = 0;

        if (channels & BitmapData_as::CHANNEL_GREEN) {
            const double g = pa(x, y, 1);
            gv = clamp(g, 0.0, 255.0);
        }
        if (channels & BitmapData_as::CHANNEL_BLUE) {
            const double b = pa(x, y, 2);
            bv = clamp(b, 0.0, 255.0);
        }
        *it = (bv | gv << 8 | rv << 16 | av << 24);
    }
    
    ptr->updateObjects();

    return as_value();
}

as_value
bitmapdata_pixelDissolve(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    UNUSED(ptr);
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
bitmapdata_scroll(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    UNUSED(ptr);
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
bitmapdata_setPixel(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (fn.nargs < 3) {
        return as_value();
    }

    const double x = toNumber(fn.arg(0), getVM(fn));
    const double y = toNumber(fn.arg(1), getVM(fn));
    if (isNaN(x) || isNaN(y) || x < 0 || y < 0) return as_value();
    if (x >= ptr->width() || y >= ptr->height()) {
        return as_value();
    }

    // Ignore any transparency here.
    const std::uint32_t color = toInt(fn.arg(2), getVM(fn));

    setPixel(*ptr, x, y, color);

    return as_value();
}

as_value
bitmapdata_setPixel32(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (fn.nargs < 3) {
        return as_value();
    }

    const double x = toNumber(fn.arg(0), getVM(fn));
    const double y = toNumber(fn.arg(1), getVM(fn));
    if (isNaN(x) || isNaN(y) || x < 0 || y < 0) return as_value();
    if (x >= ptr->width() || y >= ptr->height()) {
        return as_value();
    }

    // TODO: multiply.
    const std::uint32_t color = toInt(fn.arg(2), getVM(fn));

    setPixel32(*ptr, x, y, color);

    return as_value();
}

as_value
bitmapdata_compare(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    UNUSED(ptr);
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
bitmapdata_threshold(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    UNUSED(ptr);
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
bitmapdata_width(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    
    // Returns the immutable width of the bitmap or -1 if dispose() has
    // been called.
    if (ptr->disposed()) return -1;
    return as_value(ptr->width());
}

as_value
bitmapdata_height(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    
    // Returns the immutable height of the bitmap or -1 if dispose() has
    // been called.
    if (ptr->disposed()) return -1;
    return as_value(ptr->height());
}

as_value
bitmapdata_transparent(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    
    // Returns whether bitmap is transparent or -1 if dispose() has been called.
    if (ptr->disposed()) return -1;
    return as_value(ptr->transparent());
}

as_value
bitmapdata_rectangle(const fn_call& fn)
{
    BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    // Returns the immutable rectangle of the bitmap or -1 if dispose()
    // has been called.
    if (ptr->disposed()) return -1;

    // If it's not found construction will fail.
    as_value rectangle(findObject(fn.env(), "flash.geom.Rectangle"));
    as_function* rectCtor = rectangle.to_function();

    if (!rectCtor) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Failed to construct flash.geom.Rectangle!"));
        );
        return -1;
    }

    fn_call::Args args;
    args += 0.0, 0.0, ptr->width(), ptr->height();

    as_object* newRect = constructInstance(*rectCtor, fn.env(), args);

    return as_value(newRect);
}

as_value
bitmapdata_loadBitmap(const fn_call& fn)
{
    // This is a static method, but still requires a parent object for
    // the prototype.
    as_object* ptr = ensure<ValidThis>(fn);

    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("BitmapData.loadBitmap requires one argument"));
        );
        return as_value();
    }

    const std::string linkage = fn.arg(0).to_string();
    DisplayObject* tgt = fn.env().target();
    if (!tgt) return as_value();

    Movie* root = tgt->get_root();
    assert(root);

    const movie_definition* def = root->definition();

    const std::uint16_t id = def->exportID(linkage);
    CachedBitmap* bit = def->getBitmap(id);

    if (!bit) return as_value();

    image::GnashImage& im = bit->image();
    const size_t width = im.width();
    const size_t height = im.height();

    if (width > 2880 || height > 2880) {
        return as_value();
    }
 
    std::unique_ptr<image::GnashImage> newImage;
    if (im.type() == image::TYPE_RGBA) {
        newImage.reset(new image::ImageRGBA(width, height));
    }
    else {
        newImage.reset(new image::ImageRGB(width, height));
    }
    
    // The properties come from the 'this' object.
    Global_as& gl = getGlobal(fn);
    as_object* ret = createObject(gl);
    ret->set_member(NSV::PROP_uuPROTOuu, getMember(*ptr, NSV::PROP_PROTOTYPE));
    
    newImage->update(im.begin());
    ret->setRelay(new BitmapData_as(ret, std::move(newImage)));

    return as_value(ret);
}


as_value
get_flash_display_bitmap_data_constructor(const fn_call& fn)
{
    log_debug("Loading flash.display.BitmapData class");
    Global_as& gl = getGlobal(fn);
    as_object* proto = createObject(gl);
    attachBitmapDataInterface(*proto);
    as_object* cl = gl.createClass(&bitmapdata_ctor, proto);
    attachBitmapDataStaticProperties(*cl);
    return cl;
}

as_value
bitmapdata_ctor(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    if (fn.nargs < 2) {
        IF_VERBOSE_ASCODING_ERRORS(
             log_aserror(_("BitmapData constructor requires at least two "
                           "arguments. Will not construct a BitmapData"));
        );
        throw ActionTypeError();
    }

    const size_t width = toInt(fn.arg(0), getVM(fn));
    const size_t height = toInt(fn.arg(1), getVM(fn));
    const bool transparent = fn.nargs > 2 ? toBool(fn.arg(2), getVM(fn)) : true;
    std::uint32_t fillColor =
        fn.nargs > 3 ? toInt(fn.arg(3), getVM(fn)) : 0xffffffff;
    
    if (width > 2880 || height > 2880 || width < 1 || height < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
             log_aserror(_("BitmapData width and height must be between "
                           "1 and 2880. Will not construct a BitmapData"));
        );
        throw ActionTypeError();
    }

    std::unique_ptr<image::GnashImage> im;
    if (transparent) {
        im.reset(new image::ImageRGBA(width, height));
    }
    else {
        im.reset(new image::ImageRGB(width, height));
    }

    // There is one special case for completely transparent colours. This
    // might be a part of a more general pre-treatment as other colours
    // vary slightly, but we haven't worked it out yet.
    if (transparent && !(fillColor & 0xff000000)) fillColor = 0;

    std::fill(image::begin<image::ARGB>(*im), image::end<image::ARGB>(*im),
            fillColor);

    ptr->setRelay(new BitmapData_as(ptr, std::move(im)));

    return as_value(); 
}

void
attachBitmapDataInterface(as_object& o)
{
    const int flags = PropFlags::onlySWF8Up;

    VM& vm = getVM(o);
    o.init_member("getPixel", vm.getNative(1100, 1));
    o.init_member("setPixel", vm.getNative(1100, 2));
    o.init_member("fillRect", vm.getNative(1100, 3));
    o.init_member("copyPixels", vm.getNative(1100, 4));
    o.init_member("applyFilter", vm.getNative(1100, 5));
    o.init_member("scroll", vm.getNative(1100, 6));
    o.init_member("threshold", vm.getNative(1100, 7));
    o.init_member("draw", vm.getNative(1100, 8));
    o.init_member("pixelDissolve", vm.getNative(1100, 9));
    o.init_member("getPixel32", vm.getNative(1100, 10));
    o.init_member("setPixel32", vm.getNative(1100, 11));
    o.init_member("floodFill", vm.getNative(1100, 12));
    o.init_member("getColorBoundsRect", vm.getNative(1100, 13));
    o.init_member("perlinNoise", vm.getNative(1100, 14));
    o.init_member("colorTransform", vm.getNative(1100, 15));
    o.init_member("hitTest", vm.getNative(1100, 16));
    o.init_member("paletteMap", vm.getNative(1100, 17));
    o.init_member("merge", vm.getNative(1100, 18));
    o.init_member("noise", vm.getNative(1100, 19));
    o.init_member("copyChannel", vm.getNative(1100, 20));
    o.init_member("clone", vm.getNative(1100, 21));
    o.init_member("dispose", vm.getNative(1100, 22));
    o.init_member("generateFilterRect", vm.getNative(1100, 23));
    o.init_member("compare", vm.getNative(1100, 24));
    o.init_readonly_property("width", *vm.getNative(1100, 100), flags);
    o.init_readonly_property("height", *vm.getNative(1100, 101), flags);
    o.init_readonly_property("rectangle", *vm.getNative(1100, 102), flags);
    o.init_readonly_property("transparent", *vm.getNative(1100, 103), flags);
}

void
attachBitmapDataStaticProperties(as_object& o)
{
    VM& vm = getVM(o);

    o.init_member("loadBitmap", vm.getNative(1100, 40));

    o.init_member("RED_CHANNEL", BitmapData_as::CHANNEL_RED);
    o.init_member("GREEN_CHANNEL", BitmapData_as::CHANNEL_GREEN);
    o.init_member("BLUE_CHANNEL", BitmapData_as::CHANNEL_BLUE);
    o.init_member("ALPHA_CHANNEL", BitmapData_as::CHANNEL_ALPHA);
}
    
BitmapData_as::iterator
pixelAt(const BitmapData_as& bd, size_t x, size_t y)
{
    const size_t width = bd.width();
    if (x >= width || y >= bd.height()) return bd.end();
    return (bd.begin() + y * width + x);
}

std::uint32_t
getPixel(const BitmapData_as& bd, size_t x, size_t y)
{
    if (x >= bd.width() || y >= bd.height()) return 0;
    return *pixelAt(bd, x, y);
}

void
setPixel(const BitmapData_as& bd, size_t x, size_t y, std::uint32_t color)
{
    if (bd.disposed()) return;
    if (x >= bd.width() || y >= bd.height()) return;

    BitmapData_as::iterator it = pixelAt(bd, x, y);
    const std::uint32_t val = *it;
    *it = (color & 0xffffff) | (val & 0xff000000);
}

void
setPixel32(const BitmapData_as& bd, size_t x, size_t y, std::uint32_t color)
{
    if (bd.disposed()) return;
    if (x >= bd.width() || y >= bd.height()) return;

    BitmapData_as::iterator it = pixelAt(bd, x, y);
    *it = color;
}

void
fillRect(const BitmapData_as& bd, int x, int y, int w, int h,
        std::uint32_t color)
{
    adjustRect(x, y, w, h, bd);

    // Make sure that the rectangle has some area in the 
    // bitmap and that its bottom corner is within the
    // the bitmap.    
    if (w == 0 || h == 0) return;
    
    const size_t width = bd.width();

    BitmapData_as::iterator it = bd.begin() + y * width;
    BitmapData_as::iterator e = it + width * h;
    
    assert(e <= bd.end());

    while (it != e) {
        // Fill from x for the width of the rectangle.
        std::fill_n(it + x, w, color);
        it += width;
    }
    bd.updateObjects();
}

void
floodFill(const BitmapData_as& bd, size_t startx, size_t starty,
        std::uint32_t old, std::uint32_t fill)
{
    const size_t width = bd.width();
    const size_t height = bd.height();

    if (startx >= width || starty >= height) return;

    // We never compare alpha for RGB images.
    if (!bd.transparent()) fill |= 0xff000000;
    if (old == fill) return;

    std::queue<PixelIndexer> pixelQueue;
    pixelQueue.push(PixelIndexer(startx, starty, pixelAt(bd, startx, starty)));

    while (!pixelQueue.empty()) {

        const PixelIndexer& p = pixelQueue.front();
        const size_t x = p.x;
        const size_t y = p.y;
        BitmapData_as::iterator pix = p.pix;

        pixelQueue.pop();

        assert(pix != bd.end());

        if (*pix != old) continue;

        // Go east!
        BitmapData_as::iterator east(pix);
        if (x + 1 < width) {
            ++east;
            const BitmapData_as::iterator eaststop(pix + (width - x));
            while (east != eaststop && *east == old) ++east;
            std::fill(pix, east, fill);
        }
        size_t edone = (east - pix);
        if (!edone) ++edone;

        // Add north pixels
        if (y > 0) {
            BitmapData_as::iterator north(pix - width);
            BitmapData_as::iterator northend(north + edone);
            const size_t ny = y - 1;
            for (size_t nx = x; nx != (x + edone); ++nx, ++north) {
                if (*north == old) {
                    pixelQueue.push(PixelIndexer(nx, ny, north));
                }
            }
        }

        // Go west!
        BitmapData_as::iterator west(pix);
        if (x > 0) {
            --west;
            const BitmapData_as::iterator weststop(pix - x);
            while (west != weststop && *west == old) --west;
            std::fill(west + 1, pix, fill);
        }
        size_t wdone = (pix - west);
        if (!wdone) ++wdone;
         
        // Add south pixels
        if (y + 1 < height) {
            BitmapData_as::iterator south(pix + width);
            BitmapData_as::iterator southend(south - wdone);
            const size_t sy = y + 1;
            for (size_t sx = x; sx != x - wdone; --sx, --south) {
                if (*south == old) {
                    pixelQueue.push(PixelIndexer(sx, sy, south));
                }
            }
        }

    }

    bd.updateObjects();
}

void
adjustRect(int& x, int& y, int& w, int& h, const BitmapData_as& b) 
{
    // No negative width or height
    if (w < 0 || h < 0) {
        w = 0;
        h = 0;
        return;
    }

    // No part of rect in bitmap
    if (x >= static_cast<int>(b.width()) || y >= static_cast<int>(b.height())) {
        w = 0;
        h = 0;
        return;
    }

    // Remove left excess
    if (x < 0) {
        w += x;
        x = 0;
        if (w < 0) w = 0;
    }

    // Remove top excess
    if (y < 0) {
        h += y;
        y = 0;
        if (h < 0) h = 0;
    }

    // Remove right and bottom excess after x and y adjustments.
    w = std::min<int>(b.width() - x, w);
    h = std::min<int>(b.height() - y, h);
}


std::uint8_t
getChannel(std::uint32_t src, std::uint8_t bitmask)
{
    if (bitmask & BitmapData_as::CHANNEL_RED) {
        // Red
        return (src >> 16) & 0xff;
    }
    if (bitmask & BitmapData_as::CHANNEL_GREEN) {
        // Green
        return (src >> 8) & 0xff;
    }
    if (bitmask & BitmapData_as::CHANNEL_BLUE) {
        // Blue
        return src & 0xff;
    }
    if (bitmask & BitmapData_as::CHANNEL_ALPHA) {
        // Alpha
        return src >> 24;
    }
    return 0;
}

std::uint32_t
setChannel(std::uint32_t targ, std::uint8_t bitmask, std::uint8_t value)
{
    std::uint32_t bytemask = 0;
    std::uint32_t valmask = 0;
    if (bitmask & BitmapData_as::CHANNEL_RED) {
        // Red
        bytemask = 0xff0000;
        valmask = value << 16;
    }
    else if (bitmask & BitmapData_as::CHANNEL_GREEN) {
        // Green
        bytemask = 0xff00;
        valmask = value << 8;
    }
    else if (bitmask & BitmapData_as::CHANNEL_BLUE) {
        // Blue
        bytemask = 0xff;
        valmask = value;
    }
    else if (bitmask & BitmapData_as::CHANNEL_ALPHA) {
        // Alpha
        bytemask = 0xff000000;
        valmask = value << 24;
    }
    targ &= ~bytemask;
    targ |= valmask;
    return targ;
}


} // anonymous namespace
} // end of gnash namespace
