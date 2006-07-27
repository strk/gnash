// geometry.hpp	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some basic geometric types.


#ifndef GEOMETRY_H
#define GEOMETRY_H


#include "utility.h"
class tu_file;

class	vec3
// 3-element vector class, for 3D math.
{
public:
	float	x, y, z;	// these guys are out in public, no m_ prefix; this matches most other conventions.

	vec3() {}
	vec3(float XX, float YY, float ZZ) { x = XX; y = YY; z = ZZ; }
	vec3(const vec3& v) { x = v.x; y = v.y; z = v.z; }

	operator	const float*() const { return &x; }

	const float&	operator[](int index) const { assert(index >= 0 && index < 3); return (&x)[index]; }
	float&	operator[](int index) { assert(index >= 0 && index < 3); return (&x)[index]; }
			
	float	get(int element) const { return (&x)[element]; }
	void	set(int element, float NewValue) { (&x)[element] = NewValue; }
	float	get_x() const { return x; }
	float	get_y() const { return y; }
	float	get_z() const { return z; }
//	float&	x() { return m[0]; }
//	float&	y() { return m[1]; }
//	float&	z() { return m[2]; }
	void	set_xyz(float newx, float newy, float newz) { x = newx; y = newy; z = newz; }
	
	vec3	operator+(const vec3& v) const;
	vec3	operator-(const vec3& v) const;
	vec3	operator-() const;
	float	operator*(const vec3& v) const;
	vec3	operator*(float f) const;
	vec3	operator/(float f) const { return this->operator*(1.0f / f); }
	vec3	cross(const vec3& v) const;
	void	set_cross(const vec3& a, const vec3& b);

	// returns original length; if length is zero, sets our value to fallback.
	float	normalize(const vec3& fallback = vec3::x_axis);

	vec3&	operator=(const vec3& v) { x = v.x; y = v.y; z = v.z; return *this; }
	vec3&	operator+=(const vec3& v);
	vec3&	operator-=(const vec3& v);
	vec3&	operator*=(float f);
	vec3&	operator/=(float f) { return this->operator*=(1.0f / f); }

	float	magnitude() const;
	float	sqrmag() const;
//	float	min() const;
//	float	max() const;
//	float	minabs() const;
//	float	maxabs() const;

	void	read(tu_file* in);
	void	write(tu_file* out);

	bool	checknan() const;	// Returns true if any component is nan.

	// Some handy vector constants.
	static const vec3	zero, x_axis, y_axis, z_axis, flt_max, minus_flt_max;
};


#define INLINE_VEC3


#ifdef INLINE_VEC3


inline float	vec3::operator*(const vec3& v) const
// Dot product.
{
	float	result;
	result = x * v.x;
	result += y * v.y;
	result += z * v.z;
	return result;
}


inline vec3&	vec3::operator+=(const vec3& v)
// Adds a vec3 to *this.
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}


inline vec3&	vec3::operator-=(const vec3& v)
// Subtracts a vec3 from *this.
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}


inline void	vec3::set_cross(const vec3& a, const vec3& b)
// Cross product.
{
	assert(this != &a);
	assert(this != &b);

	x = a.y * b.z - a.z * b.y;
	y = a.z * b.x - a.x * b.z;
	z = a.x * b.y - a.y * b.x;
}


#endif // INLINE_VEC3





class	quaternion;


class	matrix
// 3x4 matrix class, for 3D transformations.
{
public:
	matrix() { set_identity(); }

	void	set_identity();
	void	set_view(const vec3& ViewNormal, const vec3& ViewUp, const vec3& ViewLocation);
	void	set_orient(const vec3& ObjectDirection, const vec3& ObjectUp, const vec3& ObjectLocation);

	static void	compose(matrix* dest, const matrix& left, const matrix& right);
	vec3	operator*(const vec3& v) const;
	matrix	operator*(const matrix& m) const;
//	operator*=(const quaternion& q);

	matrix&	operator*=(float f);
	matrix&	operator+=(const matrix& m);
	
	void	invert();
	void	invert_rotation();
	void	normalize_rotation();
	void	apply(vec3* result, const vec3& v) const;
	void	apply_rotation(vec3* result, const vec3& v) const;
	void	apply_inverse(vec3* result, const vec3& v) const;
	void	apply_inverse_rotation(vec3* result, const vec3& v) const;
	void	translate(const vec3& v);
	void	set_orientation(const quaternion& q);
	quaternion	get_orientation() const;
	
	void	set_column(int column, const vec3& v) { m[column] = v; }
	const vec3&	get_column(int column) const { return m[column]; }
private:
	vec3	m[4];
};


// class quaternion -- handy for representing rotations.

class quaternion {
public:
	quaternion() : S(1), V(vec3::zero) {}
	quaternion(const quaternion& q) : S(q.S), V(q.V) {}
	quaternion(float s, const vec3& v) : S(s), V(v) {}

	quaternion(const vec3& Axis, float Angle);	// Slightly dubious: semantics varies from other constructor depending on order of arg types.

	float	GetS() const { return S; }
	const vec3&	GetV() const { return V; }
	void	SetS(float s) { S = s; }
	void	SetV(const vec3& v) { V = v; }

	float	get(int i) const { if (i==0) return GetS(); else return V.get(i-1); }
	void	set(int i, float f) { if (i==0) S = f; else V.set(i-1, f); }

	quaternion	operator*(const quaternion& q) const;
	quaternion&	operator*=(float f) { S *= f; V *= f; return *this; }
	quaternion&	operator+=(const quaternion& q) { S += q.S; V += q.V; return *this; }

	quaternion&	operator=(const quaternion& q) { S = q.S; V = q.V; return *this; }
	quaternion&	normalize();
	quaternion&	operator*=(const quaternion& q);
	void	ApplyRotation(vec3* result, const vec3& v);
	
	quaternion	lerp(const quaternion& q, float f) const;
private:
	float	S;
	vec3	V;
};


struct plane_info {
	vec3	normal;
	float	d;

	plane_info() { }

	plane_info( const vec3& n, float dist ) {
		set( n, dist );
	}

	void	set( const vec3& n, float dist ) {
		normal = n;
		d = dist;
	}

	void	set(float nx, float ny, float nz, float dist) {
		normal.set_xyz(nx, ny, nz);
		d = dist;
	}
};


struct collision_info {
	vec3	point;
	vec3	normal;
	// float	distance, or ray_parameter;
};


namespace Geometry {
	vec3	Rotate(float Angle, const vec3& Axis, const vec3& Point);
}



#endif // GEOMETRY_H
