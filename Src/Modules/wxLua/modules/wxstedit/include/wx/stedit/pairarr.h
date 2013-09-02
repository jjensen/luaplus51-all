///////////////////////////////////////////////////////////////////////////////
// Name:        pairarr.h
// Purpose:     Sorted Key/Value pairs of wxArrays using a binary search lookup
// Author:      John Labenski
// Modified by:
// Created:     1/08/2004
// Copyright:   (c) John Labenski
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __WX_PAIRARR_H__
#define __WX_PAIRARR_H__

// --------------------------------------------------------------------------
/// class SortedPairArray - A sorted key, value pair array.
/// Values are added in sorted order by key and can be manipulated using
/// the keys or their numerical indexes.
///
/// NOTE: All functions are inlined and these header only classes dll "exported"
///       to avoid warning C4251 and then error C2491 when you "fix" the warning.
///       In other words, MSVC 2008 complains when you don't use dllexport,
///       then errors when you do if not every function is inlined.
///       More sane compilers will probably simply ignore inlining the larger functions.
///
/// To properly instantiate and forward declare these template classes use: \n
/// typedef class WXDLLIMPEXP_STEDIT SortedPairArrayNumberKey<int, wxArrayInt, wxString, wxArrayString> wxSTEPairArrayIntString; \n
// --------------------------------------------------------------------------

template <typename Tkey, typename TkeyArray, typename Tval, typename TvalArray>
class SortedPairArray
{
public:
    /// Default constructor, you must set the default value separately.
    SortedPairArray()                                    {}
    /// Typical use constructor, sets the default value.
    SortedPairArray(const Tval& defaultVal) : m_defaultValue(defaultVal) {}
    /// Copy constructor.
    SortedPairArray(const SortedPairArray& other)        { Copy(other); }
    virtual ~SortedPairArray()                           {}

    // ----------------------------------------------------------------------

    /// Returns the number of items in the array.
    inline size_t GetCount() const                       { return m_keys.size(); }
    /// Returns index of the key in the array or wxNOT_FOUND if the key is not found.
    inline int Index(const Tkey& key) const;
    /// Returns index to insert the key into the array.
    /// For append it returns GetCount() (be sure to check for GetCount()=0 case).
    /// the position to insert before, or the position of the existing key. 
    /// (see code for Add())
    inline size_t IndexForInsert(const Tkey& pos) const;
    /// Returns true if the key is in array.
    inline bool HasKey(const Tkey& key) const            { return Index(key) != wxNOT_FOUND; }
    /// Returns the value for the key or the default value if the key is not in the array.
    inline const Tval& GetValue(const Tkey& key) const;
    /// Returns a writable value for the key or the default value if the key is not in the array.
    inline Tval* GetValueMutable(const Tkey& key);
    /// Returns the value for the key or adds the key using a copy of the input newVal or the default value if newVal is NULL.
    inline Tval& GetOrCreateValue(const Tkey& key, const Tval* newVal = NULL);
    /// Returns the key at the index into the array.
    inline const Tkey& ItemKey(size_t index)   const     { return m_keys[index]; }
    /// Returns a writable key at the index into the array.
    inline Tkey& ItemKey(size_t index)                   { return m_keys[index]; }
    /// Returns the value at the index into the array.
    inline const Tval& ItemValue(size_t index) const     { return m_values[index]; }
    /// Returns a writable value at the index into the array.
    inline Tval& ItemValue(size_t index)                 { return m_values[index]; }
    /// Add the key, value pair into the array, replacing the value if the key was already added.
    inline bool Add(const Tkey& key, const Tval& value);
    /// Remove the key, value pair from the array, returns true if the key was found.
    inline bool Remove(const Tkey& key);
    /// Remove nRemove number of key, value pairs from the array at the index.
    inline void RemoveAt(size_t index, size_t nRemove = 1) { m_keys.RemoveAt(index, nRemove); m_values.RemoveAt(index, nRemove); }
    /// Remove all key, value pairs.
    inline void Clear()                                  { m_keys.Clear(); m_values.Clear(); }
    /// Copy the other pair array into this one.
    inline void Copy(const SortedPairArray& other);
    /// Get the array of keys.
    inline const TkeyArray& GetKeys()   const            { return m_keys; }
    /// Get a writable array of the keys, be sure to call Sort() if you modify any of them.
    inline TkeyArray& GetKeys()                          { return m_keys; }
    /// Get the array of values.
    inline const TvalArray& GetValues() const            { return m_values; }
    /// Get a writable array of the values.
    inline TvalArray& GetValues()                        { return m_values; }
    /// Get the default value.
    inline const Tval& GetDefaultValue() const           { return m_defaultValue; }
    /// Set the default value.
    inline void SetDefaultValue(const Tval& val)         { m_defaultValue = val; }
    /// Returns true if there are no elements in this pair array.
    inline bool IsEmpty() const                          { return GetCount() == 0; }
    /// Returns true if this pair array is equal to the other.
    inline bool IsEqualTo(const SortedPairArray& other) const;
    /// Sort the arrays by the key value.
    /// This is only needed to be called if you modify key values directly or
    /// have directly added unordered pairs using GetKeys().Add(x); GetValues().Add(y);.
    /// The pair array MUST always be sorted in order for it work properly.
    inline void Sort()                                   { if (GetCount() > 1) q_sort(0, GetCount()-1); }
    /// Copy the other pair array into this one.
    inline SortedPairArray& operator=(const SortedPairArray& other) { Copy(other); return *this; }
    /// Returns true if this pair array is equal to the other.
    inline bool operator==(const SortedPairArray& other) const { return IsEqualTo(other); }
    /// Returns true if this pair array not is equal to the other.
    inline bool operator!=(const SortedPairArray& other) const { return !IsEqualTo(other); }
    /// Returns the value at the given index.
    inline const Tval& operator[](size_t index) const    { return ItemValue(index); }
    /// Returns a writable value at the given index.
    inline Tval& operator[](size_t index)                { return ItemValue(index); }

    // ----------------------------------------------------------------------
    /// std::vector compatibility functions.
    /// @{

    /// Remove all key, value pairs.
    inline void   clear()       { Clear(); }
    /// Returns true if the array is empty.
    inline size_t empty() const { return GetCount() == 0; }
    /// Returns the number of key, value pairs.
    inline size_t size()  const { return GetCount(); }

    /// @}

protected :
    void q_sort(int left, int right);
    TkeyArray m_keys;
    TvalArray m_values;
    Tval      m_defaultValue;
};

// --------------------------------------------------------------------------
// PairArrayNumberKey
// --------------------------------------------------------------------------

template <typename Tkey, typename TkeyArray, typename Tval, typename TvalArray>
class SortedPairArrayNumberKey : public SortedPairArray<Tkey, TkeyArray, Tval, TvalArray>
{
public:
    SortedPairArrayNumberKey() : SortedPairArray<Tkey, TkeyArray, Tval, TvalArray>() {}
    SortedPairArrayNumberKey(Tval defaultVal) : SortedPairArray<Tkey, TkeyArray, Tval, TvalArray>(defaultVal) {}
    SortedPairArrayNumberKey(const SortedPairArrayNumberKey& other) : SortedPairArray<Tkey, TkeyArray, Tval, TvalArray>() { Copy(other); }
    virtual ~SortedPairArrayNumberKey() {}

    // ----------------------------------------------------------------------

    /// Call this function to shift keys after pos by adding numPos to each subsequent key.
    /// When inserting use numPos > 0 and when deleting use numPos < 0 so that 
    /// the remaining keys above pos are shifted either upwards or downwards by numpos.
    /// Note that this function will not work when Tkey is unsigned and you want to
    /// shift keys downward (numPos < 0).
    bool UpdatePos( Tkey pos, Tkey numPos );
};

// --------------------------------------------------------------------------
// class SortedPairArray - Template function implementation.
// --------------------------------------------------------------------------

template <typename Tkey, typename TkeyArray, typename Tval, typename TvalArray>
inline const Tval& SortedPairArray<Tkey, TkeyArray, Tval, TvalArray>::GetValue(const Tkey& key) const
{
    const int n = Index(key);
    if (n != wxNOT_FOUND) 
        return m_values[n];

    return m_defaultValue;
}

template <typename Tkey, typename TkeyArray, typename Tval, typename TvalArray>
inline Tval* SortedPairArray<Tkey, TkeyArray, Tval, TvalArray>::GetValueMutable(const Tkey& key)
{
    const int n = Index(key);
    if (n != wxNOT_FOUND) 
        return &m_values[n];

    return NULL;
}

template <typename Tkey, typename TkeyArray, typename Tval, typename TvalArray>
inline Tval& SortedPairArray<Tkey, TkeyArray, Tval, TvalArray>::GetOrCreateValue(const Tkey& key, const Tval* newVal)
{
    const size_t n = IndexForInsert(key);
    if (n == m_keys.GetCount())
    {
        m_keys.Add(key); 
        if (newVal != NULL)
            m_values.Add(Tval(*newVal)); 
        else
            m_values.Add(Tval(m_defaultValue)); 
    }
    else if (key != m_keys[n])
    { 
        m_keys.Insert(key, n); 
        if (newVal != NULL)
            m_values.Insert(Tval(*newVal), n); 
        else
            m_values.Insert(Tval(m_defaultValue), n); 
    }
    return m_values[n];
}

template <typename Tkey, typename TkeyArray, typename Tval, typename TvalArray>
inline bool SortedPairArray<Tkey, TkeyArray, Tval, TvalArray>::Add(const Tkey& key, const Tval& value)
{
    const size_t n = IndexForInsert(key);
    if (n == m_keys.GetCount())
    { 
        m_keys.Add(key);
        m_values.Add(value);
        return true; 
    }
    else if (key == m_keys[n])
        m_values[n] = value;
    else
    { 
        m_keys.Insert(key, n);
        m_values.Insert(value, n);
        return true; 
    }
    return false;
}

template <typename Tkey, typename TkeyArray, typename Tval, typename TvalArray>
inline bool SortedPairArray<Tkey, TkeyArray, Tval, TvalArray>::Remove(const Tkey& key)
{
    const int n = Index(key);
    if (n != wxNOT_FOUND) 
    { 
        RemoveAt(n); 
        return true; 
    }

    return false;
}

template <typename Tkey, typename TkeyArray, typename Tval, typename TvalArray>
inline int SortedPairArray<Tkey, TkeyArray, Tval, TvalArray>::Index(const Tkey& key) const
{
    size_t n, lo = 0, hi = m_keys.GetCount();
    while ( lo < hi )
    {
        n = (lo + hi)/2;
        const Tkey &tmp = m_keys[n];
        if (tmp == key) return n;
        if (tmp  > key) hi = n;
        else            lo = n + 1;
    }
    return wxNOT_FOUND;
}

template <typename Tkey, typename TkeyArray, typename Tval, typename TvalArray>
inline size_t SortedPairArray<Tkey, TkeyArray, Tval, TvalArray>::IndexForInsert(const Tkey& key) const
{
    size_t n, lo = 0, hi = m_keys.GetCount();
    while ( lo < hi )
    {
        n = (lo + hi)/2;
        const Tkey &tmp = m_keys[n];
        if (tmp == key) return n;
        if (tmp  > key) hi = n;
        else            lo = n + 1;
    }
    return lo;
}

template <typename Tkey, typename TkeyArray, typename Tval, typename TvalArray>
inline void SortedPairArray<Tkey, TkeyArray, Tval, TvalArray>::Copy(const SortedPairArray& other)
{
    m_keys         = other.GetKeys();
    m_values       = other.GetValues();
    m_defaultValue = other.GetDefaultValue();
}

template <typename Tkey, typename TkeyArray, typename Tval, typename TvalArray>
inline bool SortedPairArray<Tkey, TkeyArray, Tval, TvalArray>::IsEqualTo(const SortedPairArray& other) const
{
    if (GetCount() != other.GetCount()) 
        return false;

    size_t n, count = GetCount();
    for (n = 0; n < count; ++n)
    {
        if ((m_keys[n] != other.m_keys[n]) || (m_values[n] != other.m_values[n]))
            return false;
    }
    return true;
}

template <typename Tkey, typename TkeyArray, typename Tval, typename TvalArray>
void SortedPairArray<Tkey, TkeyArray, Tval, TvalArray>::q_sort(int left, int right)
{
    int l_hold = left, r_hold = right;

    Tkey pivot    = m_keys[left]; 
    Tval pivotVal = m_values[left];

    while (left < right)
    {
        while ((m_keys[right] >= pivot) && (left < right)) --right;

        if (left != right) 
        { 
            m_keys[left]   = m_keys[right];
            m_values[left] = m_values[right]; 
            ++left; 
        }

        while ((m_keys[left] <= pivot) && (left < right)) ++left;

        if (left != right) 
        { 
            m_keys[right]   = m_keys[left];
            m_values[right] = m_values[left]; 
            --right; 
        }
    }

    m_keys[left]   = pivot; 
    m_values[left] = pivotVal;

    if (l_hold < left) q_sort(l_hold, left-1);
    if (r_hold > left) q_sort(left+1, r_hold);
}

// --------------------------------------------------------------------------
// class PairArrayNumberKey - Template function implementation.
// --------------------------------------------------------------------------

template <typename Tkey, typename TkeyArray, typename Tval, typename TvalArray>
bool SortedPairArrayNumberKey<Tkey, TkeyArray, Tval, TvalArray>::UpdatePos( Tkey pos, Tkey numPos )
{
    int n, count = this->m_keys.GetCount(), start_idx = this->IndexForInsert(pos);
    if ((numPos == 0) || (start_idx >= count)) return false;

    if ( numPos > 0 )
    {
        for (n = start_idx; n < count; ++n)
            this->m_keys[n] += numPos;
    }
    else if ( numPos < 0 )
    {
        Tkey pos_right = pos - numPos;

        for (n = start_idx; n < count; ++n)
        {
            Tkey &k = this->m_keys[n];
            if (k < pos_right)  
            { 
                this->RemoveAt(n); 
                --n; 
                --count; 
            }
            else if (k >= pos_right)
            { 
                k += numPos; 
            }
        }
    }
    return true;
}


// ============================================================================
// ============================================================================
// Old MACRO implementation
// ============================================================================
// ============================================================================

//#define PAIR_ARRAY_USE_MACROS
#ifdef PAIR_ARRAY_USE_MACROS

// Note: there is no cpp file as all the code is contained in these macros.

#include <wx/dynarray.h>

// ============================================================================
// Provides macros for creating your own (key, value) pair arrays using a binary
// search to insert/retrieve values using a key. While this doesn't have the
// performance of a good hash table O(n), it is smaller and with a lookup speed
// O(log(n)) is suitable for some applications. You can use virtually any
// class for keys and values that can be put into wxArrays so long as they
// define the standard comparison operators ==, <, >, <=, >=.
//
// Implementation note: I've chosen to use two independent arrays instead of
// a single array of a data struct with (key, value) members to squeeze out the
// slightest increase in performance. My testing using a single class with
// key, value members and a wxSortedArray can be either as much as 2x faster
// for (int, int), but more than 2x slower for (int, bigclass).
// ----------------------------------------------------------------------------
// DECLARE_PAIRARRAY(Tkey, TkeyArray, Tval, TvalArray, name, classexp)
// DEFINE_PAIRARRAY(Tkey, Tval, name)
//   Tkey must have the operators =, ==, <, >, <=, >=
//     They'll be sorted by using the <, >, <=, >= operators
//   Tval must have a default constructor and be able to be passed as const Tval& val
//   You must have created wx(Object)Arrays of Tkey with name TkeyArray
//     and of Tval named TvalArray, for example wxArrayInt and wxArrayString
//
// Creates a class named "name" that manages the TkeyArray, TvalArray data.
//   It keeps the pairs sorted in order of Tkey and uses a binary search
//   to retrieve and set the values.
//
// ----------------------------------------------------------------------------
// DECLARE_PAIRARRAY_NUMKEY(Tkey, TkeyArray, Tval, TvalArray, name, classexp)
// DEFINE_PAIRARRAY_NUMKEY(Tkey, Tval, name)
//   This requires that Tkey has the operators +, -. It probably only makes
//   sense if they're numbers like int, long, float, double...
//   UpdatePos(Tkey pos, Tkey numpos) is added for inserting if num > 0 or
//     deleting if num < 0, the remaining keys above pos are shifted either
//     upwards or downwards by numpos.
//
// ----------------------------------------------------------------------------
// DECLARE_PAIRARRAY_INTKEY(Tval, TvalArray, name, classexp)
// DEFINE_PAIRARRAY_INTKEY(Tval, name)
//   Simple defs for when Tkey is an int (wxArrayInt), Tval may be anything.
//   UpdatePos(int pos, int numpos) is added for inserting if num > 0 or
//     deleting if num < 0, the remaining keys above pos are shifted either
//     upwards or downwards by numpos.
//
// ----------------------------------------------------------------------------
// name() - default constructor
// name(const Tval& defaultVal) - initialize the class with the default value,
//     see Get/SetDefaultValue to change it later.
// name(const name& other) - full copy constructor
// name(Tkey key, const Tval& val) - create with the first pair
// size_t GetCount() - get the number of pairs
// int Index(Tkey) - find this key returning position in pair array or wxNOT_FOUND
// size_t IndexForInsert(Tkey) - find array position to insert key at, returns
//     GetCount for append (check first in case count=0), the pos to insert
//     before, or the pos with existing key  (see Add for code)
// bool HasKey(Tkey) - does this key exist
// Tval& GetValue(Tkey) - get the value for this key or it it doesn't exist
//     the default value, see also Get/SetDefaultValue.
// Tval& GetOrCreateValue(Tkey key) - get or create a GetDefaultValue() value
//     for this key returning the created value that you can set.
// bool Add(Tkey, Tval) - set the Tval for this Tkey, replacing if exists
//                        returns true if the pair didn't previously exist
// bool Remove(Tkey) - remove pair with this Tkey, returns if it existed
// void RemoveAt(index) - remove the key and value at this array index
// void Clear() - clears the pair arrays
// const Tval& ItemValue(index) const - get the Tval at this array index
// const Tkey& ItemKey(index) const - get the Tkey at this array index
// Tval& ItemValue(index) - get the Tval at this array index
// Tkey& ItemKey(index) - get the Tkey at this array index
// TvalArray& GetValues() - get the TvalArray
// TkeyArray& GetKeys() - get the TkeyArray (don't unsort them)
// const Tval& GetDefaultValue() const - get the default value to return for
//   GetValue(Tkey) when the key doesn't exist. (inits to Tval())
// void SetDefaultValue(const Tval& val) - set the default value to return for
//   GetValue(Tkey) when the key doesn't exist. If your values don't have a
//   default constructor (eg. ints) you'll want to set this.
// void Copy(const name& other) - make full copy of other
// void Sort() - sort the pairs by the keys (only necessary if you want to
//   quickly add unorderered pairs using GetKeys().Add(x); GetValues().Add(x);)
//   You MUST keep them sorted for the lookup mechanism to work.
// name& operator=(const name& other) - make full copy of other
// Tval& operator[](size_t index) - get the value at this array index
// const Tval& operator[](size_t index) const - get the value at this array index
//
// ----------------------------------------------------------------------------
// DECLARE_PAIRARRAY_NUMKEY and DECLARE_PAIRARRAY_INTKEY - added function
// bool UpdatePos(int pos, int numPos) -
//   if numPos > 0 - shifts keys greater than pos by numPos
//   if numPos < 0 - deletes keys between pos and pos-numPos,
//     shifts keys greater than by pos-numPos by -numPos
//
// ============================================================================
// Examples:
//
// 1.) For string arrays you'll write this in the header
// DECLARE_PAIRARRAY(wxString, wxArrayString, wxString, wxArrayString,
//                   wxPairArrayStringString, class WXDLLIMPEXP_ADV)
// And this code in some cpp file.
// DEFINE_PAIRARRAY(wxString, wxString, wxPairArrayStringString)
//
// ----------------------------------------------------------------------------
// 2.) For int keys and wxString values, it's simpler, in your header
// DECLARE_PAIRARRAY_INTKEY(wxString, wxArrayString,
//                          wxPairArrayIntSheetString, class WXDLLIMPEXP_ADV)
//
// You can even make nested pair arrays, 2D arrays (wxSheetStringSparseTable)
// WX_DECLARE_OBJARRAY_WITH_DECL(wxPairArrayIntSheetString,
//                               wxArrayPairArrayIntSheetString,
//                               class WXDLLIMPEXP_ADV);
// DECLARE_PAIRARRAY_INTKEY(wxPairArrayIntSheetString, wxArrayPairArrayIntSheetString,
//                          wxPairArrayIntPairArraySheetString, class WXDLLIMPEXP_ADV)
//
// Then in some source file have
// DEFINE_PAIRARRAY_INTKEY(wxString, wxPairArrayIntSheetString)
// DEFINE_PAIRARRAY_INTKEY(wxPairArrayIntSheetString,
//                         wxPairArrayIntPairArraySheetString)
//
// ----------------------------------------------------------------------------
// 3.) For arbitrary "number" type arrays that will use the UpdatePos function
//
// Create your own "number" array, lets use double
// WX_DEFINE_USER_EXPORTED_ARRAY_DOUBLE(double, wxArrayDouble, class WXDLLIMPEXP_PLOTLIB)
// Now declare the pair array in a header file
// DECLARE_PAIRARRAY_NUMKEY( double, wxArrayDouble, wxString, wxArrayString,
//                           wxPairArrayDoubleString, class );
// and define the code for it in a source file
// DEFINE_PAIRARRAY_NUMKEY(double, wxString, wxPairArrayDoubleString);
// wxPairArrayDoubleString pairDblStr;
//
// ============================================================================

#define DECLARE_PAIRARRAY_BASE(Tkey, TkeyArray, Tval, TvalArray, name, classexp) \
\
classexp name                                                                \
{                                                                            \
public:                                                                      \
    name() {}                                                                \
    name(const Tval& defaultVal) : m_defaultValue(defaultVal) {}             \
    name(const name& other) { Copy(other); }                                 \
    name(const Tkey& key, const Tval& val) { m_keys.Add(key); m_values.Add(val); } \
    size_t GetCount() const { return m_keys.GetCount(); }                    \
    int Index(const Tkey& key) const;                                        \
    size_t IndexForInsert(const Tkey& pos) const;                            \
    bool HasKey(const Tkey& key) const { return Index(key) != wxNOT_FOUND; } \
    const Tval& GetValue(const Tkey& key) const;                             \
    Tval& GetValue(const Tkey& key);                                         \
    Tval& GetOrCreateValue(const Tkey& key);                                 \
    const Tval& ItemValue(size_t index) const { return m_values[index]; }    \
    const Tkey& ItemKey(size_t index)   const { return m_keys[index]; }      \
    Tval& ItemValue(size_t index) { return m_values[index]; }                \
    Tkey& ItemKey(size_t index)   { return m_keys[index]; }                  \
    bool Add(const Tkey& key, const Tval& value);                            \
    bool Remove(const Tkey& key);                                            \
    void RemoveAt(size_t index) { m_keys.RemoveAt(index); m_values.RemoveAt(index); } \
    void Clear() { m_keys.Clear(); m_values.Clear(); }                       \
    const TvalArray& GetValues() const { return m_values; }                  \
    const TkeyArray& GetKeys()   const { return m_keys; }                    \
    TvalArray& GetValues() { return m_values; }                              \
    TkeyArray& GetKeys()   { return m_keys; }                                \
    const Tval& GetDefaultValue() const { return m_defaultValue; }           \
    void SetDefaultValue(const Tval& val) { m_defaultValue = val; }          \
    void Copy(const name& other);                                            \
    bool IsEqualTo(const name& other) const;                                 \
    void Sort() { if (GetCount() > 1) q_sort(0, GetCount()-1); }             \
    name& operator=(const name& other) { Copy(other); return *this; }        \
    bool operator==(const name& other) const { return IsEqualTo(other); }    \
    bool operator!=(const name& other) const { return !IsEqualTo(other); }   \
    Tval& operator[](size_t index) { return ItemValue(index); }              \
    const Tval& operator[](size_t index) const { return ItemValue(index); }  \
protected :                                                                  \
    void q_sort(int left, int right);                                        \
    TkeyArray m_keys;                                                        \
    TvalArray m_values;                                                      \
    Tval m_defaultValue;

// ----------------------------------------------------------------------------
// Note: The above macros is incomplete to allow you to extend the class.

#define DECLARE_PAIRARRAY(Tkey, TkeyArray, Tval, TvalArray, name, classexp) \
DECLARE_PAIRARRAY_BASE(Tkey, TkeyArray, Tval, TvalArray, name, classexp)    \
};

#define DECLARE_PAIRARRAY_NUMKEY_BASE(Tkey, TkeyArray, Tval, TvalArray, name, classexp) \
DECLARE_PAIRARRAY_BASE(Tkey, TkeyArray, Tval, TvalArray, name, classexp) \
public: \
    bool UpdatePos( Tkey pos, Tkey numPos );

#define DECLARE_PAIRARRAY_NUMKEY(Tkey, TkeyArray, Tval, TvalArray, name, classexp) \
DECLARE_PAIRARRAY_NUMKEY_BASE(Tkey, TkeyArray, Tval, TvalArray, name, classexp)    \
};

#define DECLARE_PAIRARRAY_INTKEY(Tval, TvalArray, name, classexp) \
DECLARE_PAIRARRAY_NUMKEY_BASE(int, wxArrayInt, Tval, TvalArray, name, classexp) \
};

// ============================================================================
#define DEFINE_PAIRARRAY(Tkey, Tval, name) \
\
const Tval& name::GetValue(const Tkey& key) const \
{ \
    const int n = Index(key); \
    if (n != wxNOT_FOUND) return m_values[n]; \
    return m_defaultValue; \
} \
Tval& name::GetValue(const Tkey& key) \
{ \
    const int n = Index(key); \
    if (n != wxNOT_FOUND) return m_values[n]; \
    return m_defaultValue; \
} \
Tval& name::GetOrCreateValue(const Tkey& key) \
{ \
    const size_t n = IndexForInsert(key); \
    if (n == m_keys.GetCount())  \
        { m_keys.Add(key); m_values.Add(Tval(m_defaultValue)); } \
    else if (key != m_keys[n])  \
        { m_keys.Insert(key, n); m_values.Insert(Tval(m_defaultValue), n); } \
    return m_values[n]; \
} \
bool name::Add(const Tkey& key, const Tval& value) \
{ \
    const size_t n = IndexForInsert(key); \
    if (n == m_keys.GetCount())  \
        { m_keys.Add(key); m_values.Add(value); return true; } \
    else if (key == m_keys[n])  \
        m_values[n] = value; \
    else \
        { m_keys.Insert(key, n); m_values.Insert(value, n); return true; } \
    return false; \
} \
bool name::Remove(const Tkey& key) \
{ \
    const int n = Index(key); \
    if (n != wxNOT_FOUND) { RemoveAt(n); return true; } \
    return false; \
} \
int name::Index(const Tkey& key) const \
{ \
    size_t n, lo = 0, hi = m_keys.GetCount(); \
    while ( lo < hi ) \
    { \
        n = (lo + hi)/2;             \
        const Tkey &tmp = m_keys[n]; \
        if (tmp == key) return n;    \
        if (tmp  > key) hi = n;      \
        else            lo = n + 1;  \
    } \
    return wxNOT_FOUND; \
} \
size_t name::IndexForInsert(const Tkey& key) const \
{ \
    size_t n, lo = 0, hi = m_keys.GetCount(); \
    while ( lo < hi ) \
    { \
        n = (lo + hi)/2;             \
        const Tkey &tmp = m_keys[n]; \
        if (tmp == key) return n;    \
        if (tmp  > key) hi = n;      \
        else            lo = n + 1;  \
    } \
    return lo; \
} \
void name::Copy(const name& other) \
{ \
    m_keys = other.GetKeys();                 \
    m_values = other.GetValues();             \
    m_defaultValue = other.GetDefaultValue(); \
} \
bool name::IsEqualTo(const name& other) const \
{ \
    if (GetCount() != other.GetCount()) return false; \
    size_t n, count = GetCount(); \
    for (n = 0; n < count; ++n) \
        if ((ItemKey(n) != other.ItemKey(n)) || \
            (ItemValue(n) != other.ItemValue(n))) return false; \
    return true; \
} \
void name::q_sort(int left, int right) \
{ \
    int l_hold = left, r_hold = right; \
    Tkey pivot = m_keys[left]; Tval pivotVal = m_values[left]; \
    while (left < right) \
    { \
        while ((m_keys[right] >= pivot) && (left < right)) --right;       \
        if (left != right) { m_keys[left]   = m_keys[right];              \
                             m_values[left] = m_values[right]; ++left; }  \
        while ((m_keys[left] <= pivot) && (left < right)) ++left;         \
        if (left != right) { m_keys[right]   = m_keys[left];              \
                             m_values[right] = m_values[left]; --right; } \
    } \
    m_keys[left] = pivot; m_values[left] = pivotVal; \
    if (l_hold < left) q_sort(l_hold, left-1); \
    if (r_hold > left) q_sort(left+1, r_hold); \
}

// ----------------------------------------------------------------------------

#define DEFINE_PAIRARRAY_INTKEY(Tval, name) \
DEFINE_PAIRARRAY_NUMKEY(int, Tval, name)

#define DEFINE_PAIRARRAY_NUMKEY(Tkey, Tval, name) \
DEFINE_PAIRARRAY(Tkey, Tval, name)                \
bool name::UpdatePos( Tkey pos, Tkey numPos )     \
{ \
    int n, count = m_keys.GetCount(), start_idx = IndexForInsert(pos); \
    if ((numPos == 0) || (start_idx >= count)) return false; \
    if ( numPos > 0 ) \
    { \
        for (n = start_idx; n < count; ++n) \
            m_keys[n] += numPos; \
    } \
    else if ( numPos < 0 ) \
    { \
        Tkey pos_right = pos-numPos;    \
        for (n = start_idx; n < count; ++n) \
        { \
            Tkey &k = m_keys[n];                              \
            if (k < pos_right) { RemoveAt(n); --n; --count; } \
            else if (k >= pos_right) { k += numPos; }         \
        } \
    } \
    return true; \
}

#endif // PAIR_ARRAY_USE_MACROS

#endif  // __WX_PAIRARR_H__
