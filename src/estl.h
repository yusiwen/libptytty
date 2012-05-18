#ifndef ESTL_H_
#define ESTL_H_

#include <stdlib.h>
#include <string.h>

template<typename T, typename U> static inline T min (T a, U b) { return a < (T)b ? a : (T)b; }
template<typename T, typename U> static inline T max (T a, U b) { return a > (T)b ? a : (T)b; }

template<typename T, typename U> static inline void swap (T& a, U& b) { T t = a; a = (T)b; b = (U)t; }

template <typename I, typename T>
I find (I first, I last, const T& value)
{
  while (first != last && *first != value)
    ++first;

  return first;
}

// see ecb.h for details
#ifndef ECB_GCC_VERSION
  #if !defined __GNUC_MINOR__ || defined __INTEL_COMPILER || defined __SUNPRO_C || defined __SUNPRO_CC || defined __llvm__ || defined __clang__
    #define ECB_GCC_VERSION(major,minor) 0
  #else
    #define ECB_GCC_VERSION(major,minor) (__GNUC__ > (major) || (__GNUC__ == (major) && __GNUC_MINOR__ >= (minor)))
  #endif
#endif

#include <new>

#if __cplusplus >= 201103L
  #include <type_traits>
#endif

/* simplevec taken (and heavily modified), from:
 *
 *  MICO --- a free CORBA implementation
 *  Copyright (C) 1997-98 Kay Roemer & Arno Puder
 *  originally GPLv2 or any later
 */
template<class T>
struct simplevec
{
  typedef T *iterator;
  typedef const T *const_iterator;
  typedef unsigned long size_type;

  static bool is_simple_enough ()
  {
    #if __cplusplus >= 201103L
      return std::is_trivially_assignable<T, T>::value
          && std::is_trivially_constructable<T>::value
          && std::is_trivially_copyable<T>::value
          && std::is_trivially_destructible<T>::value;
    #elif ECB_GCC_VERSION(4,4)
      return __has_trivial_assign (T)
          && __has_trivial_constructor (T)
          && __has_trivial_copy (T)
          && __has_trivial_destructor (T);
    #else
      return 0;
    #endif
  }

private:
  size_type _last, _size;
  T *_buf;

public:
  const_iterator begin () const { return &_buf[0]; }
  iterator       begin ()       { return &_buf[0]; }

  const_iterator end () const { return &_buf[_last]; }
  iterator       end ()       { return &_buf[_last]; }

  size_type capacity () const { return _size; }
  size_type size     () const { return _last; }

private:
  static T *alloc (size_type n)
  {
    return (T *)::operator new ((size_t) (n * sizeof (T)));
  }

  void dealloc ()
  {
    if (!is_simple_enough ())
      for (size_type i = 0; i < _last; ++i)
        _buf [i].~T ();

    ::operator delete (_buf);
  }

  size_type good_size (size_type n)
  {
    return max (n, _size ? _size * 2 : 5);
  }

  // these copy helpers actually use the copy constructor, not assignment
  static void copy_lower (iterator dst, iterator src, size_type n)
  {
    if (is_simple_enough ())
      memmove (dst, src, sizeof (T) * n);
    else
      while (n--)
        new (dst++) T (*src++);
  }

  static void copy_higher (iterator dst, iterator src, size_type n)
  {
    if (is_simple_enough ())
      memmove (dst, src, sizeof (T) * n);
    else
      while (n--)
        new (dst + n) T (*(src + n));
  }

  static void copy (iterator dst, iterator src, size_type n)
  {
    if (is_simple_enough ())
      memcpy (dst, src, sizeof (T) * n);
    else
      copy_lower (dst, src, n);
  }

  void ins (iterator where, size_type n)
  {
    if (_last + n <= _size)
      copy_higher (where + n, where, end () - where);
    else
      {
        size_type sz = _last + n;
        sz = good_size (sz);
        T *nbuf = alloc (sz);

        if (_buf)
          {
            copy (nbuf, begin (), where - begin ());
            copy (nbuf + (where - begin ()) + n, where, end () - where);
            dealloc ();
          }

        _buf = nbuf;
        _size = sz;
      }
  }

public:
  void reserve (size_type sz)
  {
    if (_size < sz)
      {
        sz = good_size (sz);
        T *nbuf = alloc (sz);

        if (_buf)
          {
            copy (nbuf, begin (), _last);
            dealloc ();
          }

        _buf = nbuf;
        _size = sz;
      }
  }

  void resize (size_type sz)
  {
    reserve (sz);

    if (is_simple_enough ())
      _last = sz;
    else
      {
        while (_last < sz)
          new (_buf + _last++) T ();
        while (_last > sz)
          _buf [--_last].~T ();
      }
  }

  simplevec ()
  : _last(0), _size(0), _buf(0)
  {
  }

  simplevec (size_type n, const T& t = T ())
  : _last(0), _size(0), _buf(0)
  {
    insert (begin (), n, t);
  }

  simplevec (const_iterator first, const_iterator last)
  : _last(0), _size(0), _buf(0)
  {
    insert (begin (), first, last);
  }

  simplevec (const simplevec<T> &v)
  : _last(0), _size(0), _buf(0)
  {
    insert (begin (), v.begin (), v.end ());
  }

  simplevec<T> &operator= (const simplevec<T> &v)
  {
    if (this != &v)
      {

        dealloc ();
        _size = 0;
        _buf  = 0;
        _last = 0;
        reserve (v._last);

        copy (_buf, v.begin (), v.size ());
        _last = v._last;
      }

    return *this;
  }

  ~simplevec ()
  {
    dealloc ();
  }

  const T &front () const { return _buf[      0]; }
        T &front ()       { return _buf[      0]; }
  const T &back  () const { return _buf[_last-1]; }
        T &back  ()       { return _buf[_last-1]; }

  bool empty () const
  {
    return _last == 0;
  }

  void clear ()
  {
    _last = 0;
  }

  void push_back (const T &t)
  {
    reserve (_last + 1);
    new (_buf + _last++) T (t);
  }

  void pop_back ()
  {
    --_last;
  }

  const T &operator [](size_type idx) const { return _buf[idx]; }
        T &operator [](size_type idx)       { return _buf[idx]; }

  iterator insert (iterator pos, const T &t)
  {
    size_type at = pos - begin ();
    ins (pos, 1);
    pos = begin () + at;
    *pos = t;
    ++_last;
    return pos;
  }

  iterator insert (iterator pos, const_iterator first, const_iterator last)
  {
    size_type n  = last - first;
    size_type at = pos - begin ();

    if (n > 0)
      {
        ins (pos, n);
        _last += n;
        copy (pos, first, n);
      }

    return pos;
  }

  iterator insert (iterator pos, size_type n, const T &t)
  {
    size_type at = pos - begin ();

    if (n > 0)
      {
        ins (pos, n);
        pos = begin () + at;
        for (size_type i = 0; i < n; ++i)
          pos[i] = t;
        _last += n;
      }

    return pos;
  }

  void erase (iterator first, iterator last)
  {
    if (last != first)
      {
        if (!is_simple_enough ())
          for (iterator i = first; i < last; ++i)
            i->~T ();

        copy_lower (last, first, end () - last);

        _last -= last - first;
      }
  }

  void erase (iterator pos)
  {
    if (pos != end ())
      erase (pos, pos + 1);
  }

  void swap (simplevec<T> &t)
  {
    ::swap (_last, t._last);
    ::swap (_size, t._size);
    ::swap (_buf , t._buf );
  }
};

template<class T>
bool operator ==(const simplevec<T> &v1, const simplevec<T> &v2)
{
  if (v1.size () != v2.size ()) return false;

  return !v1.size () || !memcmp (&v1[0], &v2[0], v1.size () * sizeof (T));
}

template<class T>
bool operator <(const simplevec<T> &v1, const simplevec<T> &v2)
{
  unsigned long minlast = min (v1.size (), v2.size ());

  for (unsigned long i = 0; i < minlast; ++i)
    {
      if (v1[i] < v2[i]) return true;
      if (v2[i] < v1[i]) return false;
    }
  return v1.size () < v2.size ();
}

template<typename T>
struct vector : simplevec<T>
{
};

#endif

