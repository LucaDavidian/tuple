#ifndef TUPLE_H
#define TUPLE_H

#include "../utility/utility.hpp"
#include "../traits/traits.hpp"
#include <iostream>

/**** tuple class template definition ****/

// primary class template
template <typename... Types>
class Tuple;

template <typename Head, typename... Tail>
std::ostream &PrintTuple(std::ostream &os, Tuple<Head, Tail...> const &tuple, bool first = true);

// partial specialization (recursive implementation)
template <typename Head, typename... Tail>
class Tuple<Head, Tail...>
{
template <typename...> friend class Tuple;
friend std::ostream &PrintTuple<>(std::ostream&, const Tuple&, bool);
//friend std::ostream &PrintTuple<Head, Tail...>(std::ostream&, const Tuple&, bool);
private:
    Head mHead;
    Tuple<Tail...> mTail;
public:
    Tuple() {}
    
    Tuple(const Head &head, const Tuple<Tail...> &tail) : mHead(head), mTail(tail) {}

    // construct tuple from elements
    template <typename Head_, typename... Tail_, typename = typename Traits::EnableIf<sizeof...(Tail_) == sizeof...(Tail)>::Type>
    Tuple(Head_ &&head, Tail_ &&... tail) : mHead(Utility::Forward<Head_>(head)), mTail(Utility::Forward<Tail_>(tail)...) {} 

    // copy constructor (tuples must have same size)
    template <typename... Types, typename = typename Traits::EnableIf<sizeof...(Types) == sizeof...(Tail) + 1U>::Type>
    Tuple(const Tuple<Types...> &other) : mHead(other.mHead), mTail(other.mTail) {}

    // move constructor (tuples must have same size)
    template <typename... Types, typename = typename Traits::EnableIf<sizeof...(Types) == sizeof...(Tail) + 1U>::Type>
    Tuple(Tuple<Types...> &&other) : mHead(Utility::Move(other.mHead)), mTail(Utility::Move(other.mTail)) {}

    Head &GetHead() { return mHead; }
    const Head &GetHead() const { return mHead; }
    Tuple<Tail...> &GetTail() { return mTail; }
    const Tuple<Tail...> &GetTail() const { return mTail; }

    template <typename Head_, typename... Tail_, typename = typename Traits::EnableIf<sizeof...(Tail_) == sizeof...(Tail)>::Type>
    bool operator==(const Tuple<Head_, Tail_...> &other) const;
    template <typename Head_, typename... Tail_, typename = typename Traits::EnableIf<sizeof...(Tail_) == sizeof...(Tail)>::Type>
    bool operator!=(const Tuple<Head_, Tail_...> &other) const { return !(*this == other); }
};

template <typename Head, typename... Tail>
template <typename Head_, typename... Tail_, typename>
bool Tuple<Head, Tail...>::operator==(const Tuple<Head_, Tail_...> &other) const
{
    return mHead == other.mHead && mTail == other.mTail;
}

// base case (empty tuple)
template <>
class Tuple<>
{
public:
    bool operator==(const Tuple &other) const { return true; }
};

template <typename Head, typename... Tail>
std::ostream &PrintTuple(std::ostream &os, Tuple<Head, Tail...> const &tuple, bool first)
{
    if (first)
        os << "( ";

    os << tuple.mHead;

    if (sizeof...(Tail) != 0U)
        os << ", ";

    return PrintTuple(os, tuple.mTail, false);
}

std::ostream &PrintTuple(std::ostream &os, Tuple<> const &tuple, bool first = true)
{
    if (first)
        std::cout << "( ";
        
    std::cout << " )";

    return os;
}

template <typename... Types>
std::ostream &operator<<(std::ostream &os, Tuple<Types...> const &tuple)
{
    PrintTuple(os, tuple);

    return os;
}

/**** make tuple utility function ****/
template <typename... Types>
Tuple<typename Traits::Decay<Types>::Type...> MakeTuple(Types&&... args)
{
    return Tuple<typename Traits::Decay<Types>::Type...>(Utility::Forward<Types>(args)...);
}

/**** get function returns nth element ****/
template <unsigned int N>
struct TupleGet
{
    template <typename Head, typename... Tail>
    static auto Get(const Tuple<Head, Tail...> &tuple) -> decltype(TupleGet<N - 1>::Get(tuple.GetTail()))
    {
        return TupleGet<N - 1>::Get(tuple.GetTail());
    }

    template <typename Head, typename... Tail>
    static auto Get(Tuple<Head, Tail...> &tuple) -> decltype(TupleGet<N - 1>::Get(tuple.GetTail()))
    {
        return TupleGet<N - 1>::Get(tuple.GetTail());
    }

    template <typename Head, typename... Tail>
    static auto Get(Tuple<Head, Tail...> &&tuple) -> decltype(TupleGet<N - 1>::Get(tuple.GetTail()))
    {
        return TupleGet<N - 1>::Get(tuple.GetTail());
    }
};

template <>
struct TupleGet<0U>
{
    template <typename Head, typename... Tail>
    static Head Get(const Tuple<Head, Tail...> &tuple) { return tuple.GetHead(); }

    template <typename Head, typename... Tail>
    static Head Get(Tuple<Head, Tail...> &tuple) { return tuple.GetHead(); }

    template <typename Head, typename... Tail>
    static Head Get(Tuple<Head, Tail...> &&tuple) { return tuple.GetHead(); }
};

template <unsigned int N, typename... Types>
auto Get(const Tuple<Types...> &tuple) -> decltype(TupleGet<N>::Get(tuple))
{
    return TupleGet<N>::Get(tuple);
}

#include "../typelist/typelist.hpp"
#include "../traits/traits.hpp"

/**** typelist-like tuple algorithms ****/

template <typename... Types>
struct IsEmpty<Tuple<Types...>> : Traits::FalseType
{
    // static constexpr bool Value = false;
};

template <>
struct IsEmpty<Tuple<>> : Traits::TrueType
{
    // static constexpr bool Value = true;
};

template <typename Head, typename... Tail>
struct Front<Tuple<Head, Tail...>>
{
    using Type = Head;
};

template <typename Head, typename... Tail>
struct PopFront<Tuple<Head, Tail...>>
{
    using Type = Tuple<Tail...>;
};

template <typename... Types, typename NewType>
struct PushFront<Tuple<Types...>, NewType>
{
    using Type = Tuple<NewType, Types...>;
};

template <typename... Types, typename NewType>
struct PushBack<Tuple<Types...>, NewType>
{
    using Type = Tuple<Types..., NewType>;
};

/**** tuple algorithms ****/

// push alement to front of tuple
template <typename... Types, typename NewType>
typename PushFront<Tuple<Types...>, Traits::DecayT<NewType>>::Type PushFrontF(const Tuple<Types...> &tuple, NewType&& element) 
{
    return typename PushFront<Tuple<Types...>, Traits::DecayT<NewType>>::Type(Utility::Forward<NewType>(element), tuple);
}

// pop an element from front of tuple
template <typename... Types>
PopFrontT<Tuple<Types...>> PopFrontF(const Tuple<Types...> &tuple)
{
    return tuple.GetTail();
}

// template <typename Head, typename... Tail>
// Tuple<Tail...> PopFrontF(const Tuple<Head, Tail...> &tuple)
// {
//     return tuple.GetTail();
// }

// push an element at back of tuple
template <typename... Types, typename NewType>
Tuple<Types..., Traits::DecayT<NewType>> PushBackF(const Tuple<Types...> &tuple, NewType &&element)
{
    //return Tuple<Types..., Traits::DecayT<NewType>>(tuple.GetHead(), PushBackF(tuple.GetTail(), Utility::Forward<NewType>(element));
    return typename PushBack<Tuple<Types...>, Traits::DecayT<NewType>>::Type(tuple.GetHead(), PushBackF(tuple.GetTail(), Utility::Forward<NewType>(element)));
}

template <typename NewType>
Tuple<Traits::DecayT<NewType>> PushBackF(const Tuple<> &tuple, NewType &&element)
{
    return Tuple<NewType>(Utility::Forward<NewType>(element));
}

// reverse a tuple
template <typename Head, typename... Tail>
typename Reverse<Tuple<Head, Tail...>>::Type ReverseF(const Tuple<Head, Tail...> &tuple)
{
    return PushBackF(ReverseF(tuple.GetTail()), tuple.GetHead());
}

Tuple<> ReverseF(const Tuple<> &tuple)
{
    return tuple;
}

// reverse a tuple, index list/sequence implementation

#include "../traits/integer_sequence.hpp"

template <typename... Types, std::size_t... indices>
ReverseT<Tuple<Types...>> IndexReverseImpl(const Tuple<Types...> &tuple, Valuelist<std::size_t, indices...> vl)
{
    return MakeTuple(Get<indices>(tuple)...);
}

template <typename... Types>
ReverseT<Tuple<Types...>> IndexReverseF(const Tuple<Types...> &tuple)
{
    return IndexReverseImpl(tuple, MakeIndexListT<sizeof...(Types)>());
}

// pop element from back of tuple
template <typename... Types>
PopBackT<Tuple<Types...>> PopBackF(const Tuple<Types...> &tuple)
{
    return ReverseF(PopFrontF(ReverseF(tuple)));
}

#endif  // TUPLE_H

