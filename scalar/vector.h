/*  This file is part of the Vc library.

    Copyright (C) 2009-2013 Matthias Kretz <kretz@kde.org>

    Vc is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    Vc is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Vc.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef SCALAR_VECTOR_H
#define SCALAR_VECTOR_H

#include <assert.h>
#include <algorithm>
#include <cmath>

#ifdef _MSC_VER
#include <float.h>
#endif

#include "../common/memoryfwd.h"
#include "../common/loadstoreflags.h"
#include "types.h"
#include "mask.h"
#include "writemaskedvector.h"
#include "macros.h"

namespace Vc_VERSIONED_NAMESPACE
{
namespace Scalar
{
template <typename T> class Vector
{
    static_assert(std::is_arithmetic<T>::value,
                  "Vector<T> only accepts arithmetic builtin types as template parameter T.");

    public:
        typedef typename DetermineEntryType<T>::Type EntryType;
        using VectorEntryType = EntryType;
        using value_type = EntryType;
        using VectorType = EntryType;
        using vector_type = VectorType;

    protected:
        VectorType m_data = VectorType();

    public:
        typedef Scalar::Mask<T> Mask;
        using MaskType = Mask;
        using mask_type = Mask;
        typedef Mask MaskArgument;
        typedef Vector<T> AsArg;

        Vc_ALWAYS_INLINE VectorType &data() { return m_data; }
        Vc_ALWAYS_INLINE const VectorType &data() const { return m_data; }

        static constexpr size_t Size = 1;
        enum Constants {
            MemoryAlignment = alignof(VectorType)
        };
        typedef simdarray<int, Size, int_v, 1> IndexType;

#include "../common/generalinterface.h"

        static Vc_INTRINSIC_L Vector Random() Vc_INTRINSIC_R;

        // implict conversion from compatible Vector<U>
        template <typename U>
        Vc_INTRINSIC Vector(
            VC_ALIGNED_PARAMETER(Vector<U>) x,
            typename std::enable_if<is_implicit_cast_allowed<U, T>::value, void *>::type = nullptr)
            : m_data(static_cast<EntryType>(x.data()))
        {
        }

        // static_cast from the remaining Vector<U>
        template <typename U>
        Vc_INTRINSIC explicit Vector(
            VC_ALIGNED_PARAMETER(Vector<U>) x,
            typename std::enable_if<!is_implicit_cast_allowed<U, T>::value, void *>::type = nullptr)
            : m_data(static_cast<EntryType>(x.data()))
        {
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // broadcast
        Vc_INTRINSIC Vector(EntryType a) : m_data(a) {}
        template <typename U>
        Vc_INTRINSIC Vector(U a,
                            typename std::enable_if<std::is_same<U, int>::value &&
                                                        !std::is_same<U, EntryType>::value,
                                                    void *>::type = nullptr)
            : Vector(static_cast<EntryType>(a))
        {
        }

#include "../common/loadinterface.h"
#include "../common/storeinterface.h"

        ///////////////////////////////////////////////////////////////////////////////////////////
        // expand 1 float_v to 2 double_v                 XXX rationale? remove it for release? XXX
        template<typename OtherT> Vc_ALWAYS_INLINE void expand(Vector<OtherT> *x) const { x->data() = static_cast<OtherT>(m_data); }
        template<typename OtherT> explicit Vc_ALWAYS_INLINE Vector(const Vector<OtherT> *a) : m_data(static_cast<EntryType>(a->data())) {}

        ///////////////////////////////////////////////////////////////////////////////////////////
        // zeroing
        Vc_ALWAYS_INLINE void setZero() { m_data = 0; }
        Vc_ALWAYS_INLINE void setZero(Mask k) { if (k.data()) m_data = 0; }
        Vc_ALWAYS_INLINE void setZeroInverted(Mask k) { if (!k.data()) m_data = 0; }

        Vc_INTRINSIC_L void setQnan() Vc_INTRINSIC_R;
        Vc_INTRINSIC_L void setQnan(Mask m) Vc_INTRINSIC_R;

        ///////////////////////////////////////////////////////////////////////////////////////////
        // swizzles
        Vc_INTRINSIC const Vector<T> &abcd() const { return *this; }
        Vc_INTRINSIC const Vector<T>  cdab() const { return *this; }
        Vc_INTRINSIC const Vector<T>  badc() const { return *this; }
        Vc_INTRINSIC const Vector<T>  aaaa() const { return *this; }
        Vc_INTRINSIC const Vector<T>  bbbb() const { return *this; }
        Vc_INTRINSIC const Vector<T>  cccc() const { return *this; }
        Vc_INTRINSIC const Vector<T>  dddd() const { return *this; }
        Vc_INTRINSIC const Vector<T>  bcad() const { return *this; }
        Vc_INTRINSIC const Vector<T>  bcda() const { return *this; }
        Vc_INTRINSIC const Vector<T>  dabc() const { return *this; }
        Vc_INTRINSIC const Vector<T>  acbd() const { return *this; }
        Vc_INTRINSIC const Vector<T>  dbca() const { return *this; }
        Vc_INTRINSIC const Vector<T>  dcba() const { return *this; }

#include "../common/gatherinterface.h"
#include "../common/scatterinterface.h"

        //prefix
        Vc_ALWAYS_INLINE Vector &operator++() { ++m_data; return *this; }
        Vc_ALWAYS_INLINE Vector &operator--() { --m_data; return *this; }
        //postfix
        Vc_ALWAYS_INLINE Vector operator++(int) { return m_data++; }
        Vc_ALWAYS_INLINE Vector operator--(int) { return m_data--; }

        Vc_ALWAYS_INLINE EntryType &operator[](size_t index) {
            assert(index == 0); if(index) {}
            return m_data;
        }

        Vc_ALWAYS_INLINE EntryType operator[](size_t index) const {
            assert(index == 0); if(index) {}
            return m_data;
        }

        Vc_ALWAYS_INLINE Mask operator!() const
        {
            return Mask(!m_data);
        }
        Vc_ALWAYS_INLINE Vector operator~() const
        {
#ifndef VC_ENABLE_FLOAT_BIT_OPERATORS
            static_assert(std::is_integral<T>::value, "bit-complement can only be used with Vectors of integral type");
#endif
            return Vector(~m_data);
        }

        Vc_ALWAYS_INLINE Vector operator-() const
        {
            return -m_data;
        }
        Vc_INTRINSIC Vector Vc_PURE operator+() const { return *this; }

#define OPshift(symbol) \
        Vc_ALWAYS_INLINE Vector &operator symbol##=(const Vector &x) { m_data symbol##= x.m_data; return *this; } \
        Vc_ALWAYS_INLINE Vc_PURE Vector operator symbol(const Vector &x) const { return Vector<T>(m_data symbol x.m_data); }
        VC_ALL_SHIFTS(OPshift)
#undef OPshift

#define OP(symbol) \
        Vc_ALWAYS_INLINE Vector &operator symbol##=(const Vector &x) { m_data symbol##= x.m_data; return *this; } \
        Vc_ALWAYS_INLINE Vc_PURE Vector operator symbol(const Vector &x) const { return Vector(m_data symbol x.m_data); }
        VC_ALL_ARITHMETICS(OP)
        VC_ALL_BINARY(OP)
#undef OP

#define OPcmp(symbol) \
        Vc_ALWAYS_INLINE Vc_PURE Mask operator symbol(const Vector &x) const { return Mask(m_data symbol x.m_data); }
        VC_ALL_COMPARES(OPcmp)
#undef OPcmp

        Vc_INTRINSIC_L Vc_PURE_L Mask isNegative() const Vc_PURE_R Vc_INTRINSIC_R;

        Vc_ALWAYS_INLINE void fusedMultiplyAdd(const Vector<T> &factor, const Vector<T> &summand) {
            m_data = m_data * factor.data() + summand.data();
        }

        Vc_ALWAYS_INLINE void assign(const Vector<T> &v, const Mask &m) {
          if (m.data()) m_data = v.m_data;
        }

        template<typename V2> Vc_ALWAYS_INLINE V2 staticCast() const { return V2(static_cast<typename V2::EntryType>(m_data)); }
        template<typename V2> Vc_ALWAYS_INLINE V2 reinterpretCast() const {
            typedef typename V2::EntryType AliasT2 Vc_MAY_ALIAS;
            return V2(*reinterpret_cast<const AliasT2 *>(&m_data));
        }

        Vc_ALWAYS_INLINE WriteMaskedVector<T> operator()(Mask m) { return WriteMaskedVector<T>(this, m); }

        Vc_ALWAYS_INLINE bool pack(Mask &m1, Vector<T> &v2, Mask &m2) {
            if (!m1.data() && m2.data()) {
                m_data = v2.m_data;
                m1 = true;
                m2 = false;
                return true;
            }
            return m1;
        }

        Vc_ALWAYS_INLINE EntryType min() const { return m_data; }
        Vc_ALWAYS_INLINE EntryType max() const { return m_data; }
        Vc_ALWAYS_INLINE EntryType product() const { return m_data; }
        Vc_ALWAYS_INLINE EntryType sum() const { return m_data; }
        Vc_ALWAYS_INLINE Vector partialSum() const { return *this; }
        Vc_ALWAYS_INLINE EntryType min(Mask) const { return m_data; }
        Vc_ALWAYS_INLINE EntryType max(Mask) const { return m_data; }
        Vc_ALWAYS_INLINE EntryType product(Mask m) const
        {
            if (m.data()) {
                return m_data;
            } else {
                return EntryType(1);
            }
        }
        Vc_ALWAYS_INLINE EntryType sum(Mask m) const { if (m.data()) return m_data; return static_cast<EntryType>(0); }

        Vc_INTRINSIC Vector shifted(int amount, Vector shiftIn) const {
            VC_ASSERT(amount >= -1 && amount <= 1);
            return amount == 0 ? *this : shiftIn;
        }
        Vc_INTRINSIC Vector shifted(int amount) const { return amount == 0 ? *this : Zero(); }
        Vc_INTRINSIC Vector rotated(int) const { return *this; }
        Vc_INTRINSIC Vector reversed() const { return *this; }
        Vc_INTRINSIC Vector sorted() const { return *this; }

        template <typename F> void callWithValuesSorted(F &&f) { f(m_data); }

        template <typename F> Vc_INTRINSIC void call(F &&f) const { f(m_data); }

        template <typename F> Vc_INTRINSIC void call(F &&f, Mask mask) const
        {
            if (mask.data()) {
                f(m_data);
            }
        }

        template <typename F> Vc_INTRINSIC Vector apply(F &&f) const { return Vector(f(m_data)); }

        template <typename F> Vc_INTRINSIC Vector apply(F &&f, Mask mask) const
        {
            if (mask.data()) {
                return Vector(f(m_data));
            } else {
                return *this;
            }
        }

        template<typename IndexT> Vc_INTRINSIC void fill(EntryType (&f)(IndexT)) {
            m_data = f(0);
        }
        Vc_INTRINSIC void fill(EntryType (&f)()) {
            m_data = f();
        }

        template <typename G> static Vc_INTRINSIC Vector generate(G gen)
        {
            return gen(0);
        }

        Vc_INTRINSIC_L Vector copySign(Vector reference) const Vc_INTRINSIC_R;
        Vc_INTRINSIC_L Vector exponent() const Vc_INTRINSIC_R;

        Vc_INTRINSIC Vector interleaveLow(Vector) const { return *this; }
        Vc_INTRINSIC Vector interleaveHigh(Vector x) const { return x; }
};
template<typename T> constexpr size_t Vector<T>::Size;

template<typename T> class SwizzledVector : public Vector<T> {};

#ifdef _MSC_VER
  template<typename T> static Vc_ALWAYS_INLINE void forceToRegisters(const Vector<T> &) {
  }
#else
  template<typename T> static Vc_ALWAYS_INLINE void forceToRegisters(const Vector<T> &x01) {
      __asm__ __volatile__(""::"r"(x01.data()));
  }
  template<> Vc_ALWAYS_INLINE void forceToRegisters(const Vector<float> &x01) {
      __asm__ __volatile__(""::"x"(x01.data()));
  }
  template<> Vc_ALWAYS_INLINE void forceToRegisters(const Vector<double> &x01) {
      __asm__ __volatile__(""::"x"(x01.data()));
  }
#endif
  template<typename T1, typename T2> static Vc_ALWAYS_INLINE void forceToRegisters(
      const Vector<T1> &x01, const Vector<T2> &x02) {
      forceToRegisters(x01);
      forceToRegisters(x02);
  }
  template<typename T1, typename T2, typename T3> static Vc_ALWAYS_INLINE void forceToRegisters(
        const Vector<T1>  &,  const Vector<T2>  &, const Vector<T3>  &) {}
  template<typename T1, typename T2, typename T3, typename T4> static Vc_ALWAYS_INLINE void forceToRegisters(
        const Vector<T1>  &,  const Vector<T2>  &,
        const Vector<T3>  &,  const Vector<T4>  &) {}
  template<typename T1, typename T2, typename T3, typename T4, typename T5>
    static Vc_ALWAYS_INLINE void forceToRegisters(
        const Vector<T1>  &,  const Vector<T2>  &,
        const Vector<T3>  &,  const Vector<T4>  &,
        const Vector<T5>  &) {}
  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    static Vc_ALWAYS_INLINE void forceToRegisters(
        const Vector<T1>  &,  const Vector<T2>  &,
        const Vector<T3>  &,  const Vector<T4>  &,
        const Vector<T5>  &,  const Vector<T6>  &) {}
  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
    typename T7>
    static Vc_ALWAYS_INLINE void forceToRegisters(
        const Vector<T1>  &,  const Vector<T2>  &,
        const Vector<T3>  &,  const Vector<T4>  &,
        const Vector<T5>  &,  const Vector<T6>  &,
        const Vector<T7>  &) {}
  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
    typename T7, typename T8>
    static Vc_ALWAYS_INLINE void forceToRegisters(
        const Vector<T1>  &,  const Vector<T2>  &,
        const Vector<T3>  &,  const Vector<T4>  &,
        const Vector<T5>  &,  const Vector<T6>  &,
        const Vector<T7>  &,  const Vector<T8>  &) {}
  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
    typename T7, typename T8, typename T9>
    static Vc_ALWAYS_INLINE void forceToRegisters(
        const Vector<T1>  &,  const Vector<T2>  &,
        const Vector<T3>  &,  const Vector<T4>  &,
        const Vector<T5>  &,  const Vector<T6>  &,
        const Vector<T7>  &,  const Vector<T8>  &,
        const Vector<T9>  &) {}
  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
    typename T7, typename T8, typename T9, typename T10>
    static Vc_ALWAYS_INLINE void forceToRegisters(
        const Vector<T1>  &, const Vector<T2>  &,
        const Vector<T3>  &, const Vector<T4>  &,
        const Vector<T5>  &, const Vector<T6>  &,
        const Vector<T7>  &, const Vector<T8>  &,
        const Vector<T9>  &, const Vector<T10> &) {}
  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
    typename T7, typename T8, typename T9, typename T10, typename T11>
    static Vc_ALWAYS_INLINE void forceToRegisters(
        const Vector<T1>  &, const Vector<T2>  &,
        const Vector<T3>  &, const Vector<T4>  &,
        const Vector<T5>  &, const Vector<T6>  &,
        const Vector<T7>  &, const Vector<T8>  &,
        const Vector<T9>  &, const Vector<T10> &,
        const Vector<T11> &) {}
  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
    typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
    static Vc_ALWAYS_INLINE void forceToRegisters(
        const Vector<T1>  &, const Vector<T2>  &,
        const Vector<T3>  &, const Vector<T4>  &,
        const Vector<T5>  &, const Vector<T6>  &,
        const Vector<T7>  &, const Vector<T8>  &,
        const Vector<T9>  &, const Vector<T10> &,
        const Vector<T11> &, const Vector<T12> &) {}
  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
    typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
    static Vc_ALWAYS_INLINE void forceToRegisters(
        const Vector<T1>  &, const Vector<T2>  &,
        const Vector<T3>  &, const Vector<T4>  &,
        const Vector<T5>  &, const Vector<T6>  &,
        const Vector<T7>  &, const Vector<T8>  &,
        const Vector<T9>  &, const Vector<T10> &,
        const Vector<T11> &, const Vector<T12> &,
        const Vector<T13> &) {}
  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
    typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13,
    typename T14> static Vc_ALWAYS_INLINE void forceToRegisters(
        const Vector<T1>  &, const Vector<T2>  &,
        const Vector<T3>  &, const Vector<T4>  &,
        const Vector<T5>  &, const Vector<T6>  &,
        const Vector<T7>  &, const Vector<T8>  &,
        const Vector<T9>  &, const Vector<T10> &,
        const Vector<T11> &, const Vector<T12> &,
        const Vector<T13> &, const Vector<T14> &) {}
  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
    typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13,
    typename T14, typename T15> static Vc_ALWAYS_INLINE void forceToRegisters(
        const Vector<T1>  &, const Vector<T2>  &,
        const Vector<T3>  &, const Vector<T4>  &,
        const Vector<T5>  &, const Vector<T6>  &,
        const Vector<T7>  &, const Vector<T8>  &,
        const Vector<T9>  &, const Vector<T10> &,
        const Vector<T11> &, const Vector<T12> &,
        const Vector<T13> &, const Vector<T14> &,
        const Vector<T15> &) {}
  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
    typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13,
    typename T14, typename T15, typename T16> static Vc_ALWAYS_INLINE void forceToRegisters(
        const Vector<T1>  &, const Vector<T2>  &,
        const Vector<T3>  &, const Vector<T4>  &,
        const Vector<T5>  &, const Vector<T6>  &,
        const Vector<T7>  &, const Vector<T8>  &,
        const Vector<T9>  &, const Vector<T10> &,
        const Vector<T11> &, const Vector<T12> &,
        const Vector<T13> &, const Vector<T14> &,
        const Vector<T15> &, const Vector<T16> &) {}

#define Vc_CONDITIONAL_ASSIGN(name__, op__)                                              \
    template <Operator O, typename T, typename M, typename U>                            \
    Vc_INTRINSIC enable_if<O == Operator::name__, void> conditional_assign(              \
        Vector<T> &lhs, M &&mask, U &&rhs)                                               \
    {                                                                                    \
        if (mask.isFull()) {                                                             \
            lhs op__ std::forward<U>(rhs);                                               \
        }                                                                                \
    }
Vc_CONDITIONAL_ASSIGN(          Assign,  =)
Vc_CONDITIONAL_ASSIGN(      PlusAssign, +=)
Vc_CONDITIONAL_ASSIGN(     MinusAssign, -=)
Vc_CONDITIONAL_ASSIGN(  MultiplyAssign, *=)
Vc_CONDITIONAL_ASSIGN(    DivideAssign, /=)
Vc_CONDITIONAL_ASSIGN( RemainderAssign, %=)
Vc_CONDITIONAL_ASSIGN(       XorAssign, ^=)
Vc_CONDITIONAL_ASSIGN(       AndAssign, &=)
Vc_CONDITIONAL_ASSIGN(        OrAssign, |=)
Vc_CONDITIONAL_ASSIGN( LeftShiftAssign,<<=)
Vc_CONDITIONAL_ASSIGN(RightShiftAssign,>>=)
#undef Vc_CONDITIONAL_ASSIGN

#define Vc_CONDITIONAL_ASSIGN(name__, expr__)                                            \
    template <Operator O, typename T, typename M>                                        \
    Vc_INTRINSIC enable_if<O == Operator::name__, Vector<T>> conditional_assign(         \
        Vector<T> &lhs, M &&mask)                                                        \
    {                                                                                    \
        return mask.isFull() ? (expr__) : lhs;                                           \
    }
Vc_CONDITIONAL_ASSIGN(PostIncrement, lhs++)
Vc_CONDITIONAL_ASSIGN( PreIncrement, ++lhs)
Vc_CONDITIONAL_ASSIGN(PostDecrement, lhs--)
Vc_CONDITIONAL_ASSIGN( PreDecrement, --lhs)
#undef Vc_CONDITIONAL_ASSIGN

}  // namespace Scalar
using Scalar::conditional_assign;
}  // namespace Vc

#include "vector.tcc"
#include "undomacros.h"

#endif // SCALAR_VECTOR_H
