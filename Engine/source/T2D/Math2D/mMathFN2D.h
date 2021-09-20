#ifndef _MMATHFN2D_H_
#define _MMATHFN2D_H_

#ifndef _MMATHFN_H_
#include "math/mMathFn.h"
#endif

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif

#ifndef _MPOINT2_H_
#include "math/mPoint2.h"
#endif

extern MRandomLCG gRandomGenerator;

/// Tolerate Less-Than?
inline bool mLessThan(const F32& a, const F32& b) { return a < b; }
/// Tolerate Greater-Than?
inline bool mGreaterThan(const F32& a, const F32& b) { return a > b; }
/// Tolerate Is Equal within Range?
inline bool mIsEqualRange(const F32& a, const F32& b, const F32 epsilon = FLT_EPSILON) { return mFabs(a - b) <= epsilon; }
/// Random Float Range.
inline F32 mGetRandomF(F32 from, F32 to) { return gRandomGenerator.randF(from, to); }
/// Swap.
inline void mSwap(F32& a, F32& b) { F32 temp = b; b = a; a = temp; }

#endif
