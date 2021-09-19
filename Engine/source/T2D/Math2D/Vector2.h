#ifndef _VECTOR2_H_
#define _VECTOR2_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif // !_CONSOLETYPES_H_

#ifndef _MMATHFN2D_H_
#include "T2D/Math2D/mMathFN2D.h"
#endif

#ifndef _SIM_H_
#include "console/sim.h"
#endif // !_SIM_H_

#ifndef BOX2D_H
#include "box2D/Box2D.h"
#endif

#ifndef B2_COLLISION_H
#include "box2D/b2Collision.h"
#endif

///-----------------------------------------------------------------------------

struct Vector2 : b2Vec2
{
public:
   /// unlike box2d we want a default constructor.
   inline Vector2() : b2Vec2(0.0f, 0.0f) {}
   inline Vector2(const Vector2 & _copy)     : b2Vec2(_copy.x, _copy.y) {}
   inline Vector2(const F32 x, const F32 y)  : b2Vec2(x, y) {}
   inline Vector2(const b2Vec2 & vec2)       : b2Vec2(vec2) {}
   inline Vector2(const Point2I & point)     : b2Vec2( F32(point.x), F32(point.y) ) {}
   inline Vector2(const Point2F & point)     : b2Vec2(point.x, point.y) {}
   inline Vector2(const Point2D & point)     : b2Vec2(F32(point.x), F32(point.y)) {}


   void convolve(const Vector2&);
   void convolveInverse(const Vector2&);
   F32  len() const;

   void setMin(const Vector2 & _test);
   void setMax(const Vector2 & _test);

   void set(F32 _x, F32 _y);
   void set(const Vector2 & copy);

   /// Operators.
   inline Vector2& operator /= (const F32 s) { x /= s; y /= s; return *this; }
   inline Vector2& operator += (const Vector2& v) { x += v.x; y += v.y;   return *this; }
   inline Vector2& operator -= (const Vector2& v) { x -= v.x; y -= v.y;   return *this; }
   inline Vector2 operator / (F32 s) const { return Vector2(x / s, y / s); }
   inline Vector2 operator + (const Vector2 &v) const { return Vector2(x + v.x, y + v.y); }
   inline Vector2 operator - (const Vector2 &v) const { return Vector2(x - v.x, y - v.y); }
   friend Vector2 operator * (F32 s, const Vector2& v) { return Vector2(v.x*s, v.y*s); }
   friend Vector2 operator * (const Vector2& v, F32 s) { return Vector2(v.x*s, v.y*s); }
   friend Vector2 operator * (const Vector2& v1, Vector2& v2) { return Vector2(v1.x*v2.x, v1.y*v2.y); }
   friend Vector2 operator * (const Vector2& v1, const Vector2& v2) { return Vector2(v1.x*v2.x, v1.y*v2.y); }
   inline Vector2 operator - (void) const { return Vector2(-x, -y); }
   inline bool operator == (const Vector2 &v) const { return (v.x == x && v.y == y); }
   inline bool operator != (const Vector2 &v) const { return (v.x != x || v.y != y); }

   /// Operator 'Point2F' Support (Assignment/Conversion).
   inline Vector2 operator = (const Point2F &p) { x = p.x; y = p.y; return *this; }
   inline Vector2 operator = (const Point2I &p) { x = F32(p.x); y = F32(p.y); return *this; }
   inline operator Point2F () { return Point2F(x, y); }
   inline Point2F ToPoint2F(void) const { return Point2F(x, y); }

   inline F32& operator[](U32);
   inline const F32& operator[](U32) const;
   F32& operator[](S32 i) { return operator[](U32(i)); }
   const F32& operator[](S32 i) const { return operator[](U32(i)); }

   /// Operator 'b2Vec2' Support (Assignment/Conversions).
   inline Vector2 operator = (const b2Vec2 &p) { x = p.x; y = p.y; return *this; }

   /// Utility.
   inline void setAngle(const F32 radians) { x = mCos(radians); y = mSin(radians); }
   inline void setPolar(const F32 radians, F32 length) { x = mCos(radians)*length; y = mSin(radians)*length; }
   inline Vector2 getVecFromAng(Vector2 curPos, F32 radians, F32 length) { F32 o = mSin(radians)*length; F32 a = mCos(radians)*length; return curPos + Vector2(a, o); }

   inline const Vector2& setZero() { (*this) = getZero(); return *this; }
   inline const Vector2& setOne() { (*this) = getOne(); return *this; }
   inline static const Vector2& getZero() { static const Vector2 v(0.0f, 0.0f); return v; }
   inline static const Vector2& getOne() { static const Vector2 v(1.0f, 1.0f); return v; }
   inline F32 getAngle(void) const { return mAtan2(x, y); }
   inline F32 getMinorAxis(void) const { return mLessThan(x, y) ? x : y; }
   inline F32 getMajorAxis(void) const { return mGreaterThan(x, y) ? x : y; }
   inline Vector2 getUnitDirection(void) const { Vector2 temp(*this); temp.Normalize(); return temp; }
   inline bool isNAN(void) const { return IsValid(); }
   inline bool isEqualRange(const Vector2& v, const F32 epsilon) const { return mIsEqualRange(x, v.x, epsilon) && mIsEqualRange(y, v.y, epsilon); }
   inline bool isEqual(const Vector2& v) const { return mIsEqual(x, v.x) && mIsEqual(y, v.y); }
   inline bool notEqual(const Vector2& v) const { return !isEqual(v); }
   inline bool isXZero(void) const { return mIsZero(x); }
   inline bool isYZero(void) const { return mIsZero(y); }
   inline bool isZero(void) const { return mIsZero(LengthSquared()); }
   inline bool notZero(void) const { return !isZero(); }
   inline F32 Normalize(void) { return b2Vec2::Normalize(); }
   inline F32 Normalize(const F32 s) { const F32 length = Length(); if (length > 0.0f) m_point2F_normalize_f((F32*)this, s); return length; }
   inline Vector2& absolute(void) { if (x < 0.0f) x = -x; if (y < 0.0f) y = -y; return *this; }
   inline Vector2& receiprocate(void) { x = 1.0f / x; y = 1.0f / y; return *this; }
   inline Vector2 getReceiprocate(void) const { Vector2 temp = *this; temp.receiprocate(); return temp; }
   inline Vector2& add(const Vector2& v) { x += v.x; y += v.y; return *this; }
   inline Vector2& sub(const Vector2& v) { x -= v.x; y -= v.y; return *this; }
   inline Vector2& mult(const Vector2& v) { x *= v.x; y *= v.y; return *this; }
   inline Vector2& div(const Vector2& v) { x /= v.x; y /= v.y; return *this; }
   inline Vector2& scale(F32 scale) { x *= scale; y *= scale; return *this; }
   inline Vector2& scale(const Vector2& v) { x *= v.x; y *= v.y; return *this; }
   inline Vector2& rotate(F32 angle) { F32 tempX = x; x = x * mCos(angle) - y * mSin(angle); y = tempX * mSin(angle) + y * mCos(angle); return *this; }
   inline Vector2& rotate(const Vector2& center, F32 angle) { Vector2 temp = *this - center; temp.rotate(angle); *this = center + temp; return *this; }
   inline Vector2& perp(void) { const F32 temp = x; x = -y; y = temp; return *this; }
   inline Vector2 getPerp(void) const { return Vector2(*this).perp(); }
   inline F32 dot(const Vector2&v) { return (x * v.x) + (y * v.y); }
   inline void lerp(const Vector2& v, const F32 time, Vector2& out) { out.Set(x + (v.x - x)*time, y + (v.y - y)*time); }
   inline void swap(Vector2& v) { mSwap(v.x, x); mSwap(v.y, y); }
   inline Vector2& clamp(const Vector2& min, const Vector2& max) { x = (x < min.x) ? min.x : (x > max.x) ? max.x : x; y = (y < min.y) ? min.y : (y > max.y) ? max.y : y; return *this; }
   inline Vector2& clampZero(void) { if (isXZero()) x = 0.0f; if (isYZero()) y = 0.0f; return *this; }
   inline Vector2& clampMin(const Vector2& min) { if (x < min.x) x = min.x; if (y < min.y) y = min.y;  return *this; }
   inline Vector2& clampMax(const Vector2& max) { if (x > max.x) x = max.x; if (y > max.y) y = max.y;  return *this; }
   inline void rand(const Vector2& min, const Vector2& max) { x = mGetRandomF(min.x, max.x), y = mGetRandomF(min.y, max.y); }
   inline void round(const F32 epsilon = FLT_EPSILON) { x = mRound(x, epsilon); y = mRound(y, epsilon); }

   inline StringTableEntry stringThis(void) const { char buffer[32]; dSprintf(buffer, 32, "%g %g", x, y); return StringTable->insert(buffer); }
   inline const char* scriptThis(void) const { char* pBuffer = Con::getReturnBuffer(32); dSprintf(pBuffer, 32, "%.5g %.5g", x, y); return pBuffer; }


};

DefineConsoleType(TypeVector2, Vector2)

inline F32& Vector2::operator[](U32 index)
{
   // we only have 2 in a vector2, safety required.
   if (index > 1)
   {
      // they want to return something set it to 1
      index = 1;
   }

   if (index == 0)
      return x;

   if (index == 1)
      return y;
}

inline const F32& Vector2::operator[](U32 index) const
{
   // we only have 2 in a vector2, safety required.
   if (index > 1)
   {
      // they want to return something set it to 1
      index = 1;
   }

   if (index == 0)
      return x;

   if (index == 1)
      return y;
}

inline void Vector2::convolve(const Vector2& c)
{
   x *= c.x;
   y *= c.y;
}

inline void Vector2::convolveInverse(const Vector2& c)
{
   x /= c.x;
   y /= c.y;
}

inline F32 Vector2::len() const
{
   return mSqrt(F32(x*x + y*y));
}

inline void Vector2::setMin(const Vector2& _test)
{
   x = (_test.x < x) ? _test.x : x;
   y = (_test.y < y) ? _test.y : y;
}

inline void Vector2::setMax(const Vector2& _test)
{
   x = (_test.x > x) ? _test.x : x;
   y = (_test.y > y) ? _test.y : y;
}

inline void Vector2::set(F32 _x, F32 _y)
{
   x = _x;
   y = _y;
}

inline void Vector2::set(const Vector2& copy)
{
   x = copy.x;
   y = copy.y;
}

class BoxVec2
{
public:
   Vector2 minExtents;
   Vector2 maxExtents;
   BoxVec2() {}
   BoxVec2(const Vector2& in_rMin, const Vector2& in_rMax, const bool in_overrideCheck = false);
   BoxVec2(const F32 &xMin, const F32 &ymin,
           const F32 &xMax, const F32 &ymax);

   BoxVec2(F32 boxSize);

   void set(const Vector2& in_rMin, const Vector2& in_rMax);
   void set(const F32 &xMin, const F32 &ymin,
            const F32 &xMax, const F32 &ymax);
   void set(const Vector2& in_Length);
   void setCenter(const Vector2& center);
   Vector2 getCenter() const;
   void getCenter(Vector2* center) const;
   bool isContained(const Vector2& in_rContained) const;
   bool isOverlapped(const BoxVec2& in_rOverlap) const;
   bool isContained(const BoxVec2& in_rContain) const;

   bool isValidBox2D() const { return  (minExtents.x <= maxExtents.x) &&
                                       (minExtents.y <= maxExtents.y); }
   
   /// Returns the length of the x extent.
   F32 len_x() const { return maxExtents.x - minExtents.x; }

   /// Returns the length of the y extent.
   F32 len_y() const { return maxExtents.y - minExtents.y; }

   /// Returns the minimum box extent.
   F32 len_min() const { return getMin(len_x(), len_y()); }

   /// Returns the maximum box extent.
   F32 len_max() const { return getMax(len_x(), len_y()); }

   /// Returns the diagonal box length.
   F32 len() const { return (maxExtents - minExtents).len(); }

   /// Returns the length of extent by axis index.
   ///
   /// @param axis The axis index of 0 for x, 1 for y
   ///
   F32 len(S32 axis) const { return maxExtents[axis] - minExtents[axis]; }

   bool isEmpty() const { return len_x() <= 0.0f || len_y() <= 0.0f; }

   Vector2 getExtents() const { return maxExtents - minExtents; }


};

inline BoxVec2::BoxVec2(const Vector2& in_rMin, const Vector2& in_rMax, const bool in_overrideCheck)
   : minExtents(in_rMin),
     maxExtents(in_rMax)
{
   if (in_overrideCheck == false) {
      minExtents.setMin(in_rMax);
      maxExtents.setMax(in_rMin);
   }
}

inline BoxVec2::BoxVec2(const F32 &xMin, const F32 &yMin,
   const F32 &xMax, const F32 &yMax)
   : minExtents(xMin, yMin),
     maxExtents(xMax, yMax)
{
}

inline BoxVec2::BoxVec2(F32 boxSize)
   : minExtents(-0.5f * boxSize, -0.5f * boxSize),
     maxExtents(0.5f * boxSize, 0.5f * boxSize)
{
}

inline void BoxVec2::set(const Vector2& in_rMin, const Vector2& in_rMax)
{
   minExtents.set(in_rMin);
   maxExtents.set(in_rMax);
}

inline void BoxVec2::set(const F32 &xMin, const F32 &yMin,
                         const F32 &xMax, const F32 &yMax)
{
   minExtents.set(xMin, yMin);
   maxExtents.set(xMax, yMax);
}

inline void BoxVec2::set(const Vector2& in_Length)
{
   minExtents.set(-in_Length.x * 0.5f, -in_Length.y * 0.5f);
   maxExtents.set(in_Length.x * 0.5f, in_Length.y * 0.5f);
}

inline void BoxVec2::setCenter(const Vector2& center)
{
   F32 halflenx = len_x() * 0.5f;
   F32 halfleny = len_y() * 0.5f;

   minExtents.set(center.x - halflenx, center.y - halfleny);
   maxExtents.set(center.x + halflenx, center.y + halfleny);
}

inline Vector2 BoxVec2::getCenter() const
{
   Vector2 center;
   center.x = (minExtents.x + maxExtents.x) * 0.5f;
   center.y = (minExtents.y + maxExtents.y) * 0.5f;
   return center;
}

inline void BoxVec2::getCenter(Vector2* center) const
{
   center->x = (minExtents.x + maxExtents.x) * 0.5f;
   center->y = (minExtents.y + maxExtents.y) * 0.5f;
}

inline bool BoxVec2::isContained(const Vector2& in_rContained) const
{
   return (in_rContained.x >= minExtents.x && in_rContained.x < maxExtents.x) &&
          (in_rContained.y >= minExtents.y && in_rContained.y < maxExtents.y);
}

inline bool BoxVec2::isOverlapped(const BoxVec2& in_rOverlap) const
{
   if (in_rOverlap.minExtents.x > maxExtents.x ||
      in_rOverlap.minExtents.y > maxExtents.y)
      return false;
   if (in_rOverlap.maxExtents.x < minExtents.x ||
      in_rOverlap.maxExtents.y < minExtents.y)
      return false;
   return true;
}

inline bool BoxVec2::isContained(const BoxVec2& in_rContained) const
{
   return (minExtents.x <= in_rContained.minExtents.x) &&
          (minExtents.y <= in_rContained.minExtents.y) &&
          (maxExtents.x >= in_rContained.maxExtents.x) &&
          (maxExtents.y >= in_rContained.maxExtents.y);
}


#endif // !_VECTOR2_H_

