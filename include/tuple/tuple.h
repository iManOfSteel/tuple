#pragma once

#include <memory>
#include <tuple>

template<typename... Ts> class Tuple{};

template<>
class Tuple<>{
public:
    void swap(Tuple<> other){}
};

template<size_t,typename...> struct elem_type_holder;

template<typename T, typename... Ts>
struct elem_type_holder<0, T, Ts...> {
    typedef T type;
};

template<size_t k, typename T, typename... Ts>
struct elem_type_holder<k, T, Ts...>{
    typedef typename elem_type_holder<k - 1, Ts...>::type type;
};

template<size_t k> class getter;

template<typename T, typename... Ts>
class Tuple<T, Ts...>: public Tuple<Ts...>{
public:
    template <typename...>
    friend class Tuple;

    constexpr Tuple(): Tuple<Ts...>(), tail() {}

    explicit constexpr Tuple(const T& value, const Ts&... ts): Tuple<Ts...>(ts...), tail(value) {}

    template <typename U, typename...Us>
    explicit constexpr Tuple(U&& value, Us&&... us): Tuple<Ts...>(std::forward<Us>(us)...), tail(std::forward<U>(value)) {}

    constexpr Tuple(const Tuple<T, Ts...>& other): tail(other.tail), Tuple<Ts...>(other) {}

    constexpr Tuple(Tuple<T, Ts...>&& other): tail(std::forward<T>(other.tail)), Tuple<Ts...>(std::forward<Tuple<Ts...>>(other)) {}

    Tuple<Ts...>& parent(Tuple<T, Ts...>& tuple) {
        return *static_cast<Tuple<Ts...>*>(&tuple);
    }

    const Tuple<Ts...>& parent(const Tuple<T, Ts...>& tuple) const{
        return *static_cast<const Tuple<Ts...>*>(&tuple);
    }

    Tuple<T, Ts...>& operator=(const Tuple<T, Ts...>& other) {
        tail = other.tail;
        parent(*this) = other;
        return *this;
    };

    Tuple<T, Ts...>&operator=(Tuple<T, Ts...>&& other) {
        tail = std::move(other.tail);
        parent(*this) = std::move(other);
        return *this;
    };

    void swap(Tuple<T, Ts...>& other){
        std::swap(tail, other.tail);
        parent(*this).swap(other.parent(other));
    }

    T tail;
};


template <typename T>
struct unwrap_refwrapper
{
    using type = T;
};

template <typename T>
struct unwrap_refwrapper<std::reference_wrapper<T>>
{
    using type = T&;
};

template <typename T>
using special_decay_t = typename unwrap_refwrapper<typename std::decay<T>::type>::type;


template<typename... Ts>
constexpr auto makeTuple(Ts&&... ts){
    return Tuple<special_decay_t<Ts>...>(std::forward<Ts>(ts)...);
};


template<size_t k>
class Getter {
public:
    template<typename T, typename... Ts>
    constexpr static typename elem_type_holder<k, T, Ts...>::type &get(Tuple<T, Ts...> &tuple) {
        return Getter<k - 1>::get(tuple.parent(tuple));
    };

    template<typename T, typename... Ts>
    constexpr static const typename elem_type_holder<k, T, Ts...>::type &get(const Tuple<T, Ts...> &tuple) {
        return Getter<k - 1>::get(tuple.parent(tuple));
    };

    template<typename T, typename... Ts>
    constexpr static typename elem_type_holder<k, T, Ts...>::type get(Tuple<T, Ts...> &&tuple) {
        return Getter<k - 1>::get(std::move(tuple.parent(tuple)));
    };
};

template<>
class Getter<0> {
public:
    template<typename T, typename... Ts>
    constexpr static T &get(Tuple<T, Ts...> &tuple) {
        return tuple.tail;
    };

    template<typename T, typename... Ts>
    constexpr static const T &get(const Tuple<T, Ts...> &tuple) {
        return tuple.tail;
    };

    template<typename T, typename... Ts>
    constexpr static T get(Tuple<T, Ts...> &&tuple) {
        return std::forward<T>(tuple.tail);
    };
};

// get by index
template<size_t k, typename... Ts>
constexpr typename elem_type_holder<k, Ts...>::type& get(Tuple<Ts...>& tuple){
    return Getter<k>::get(tuple);
};

template<size_t k, typename... Ts>
constexpr const typename elem_type_holder<k, Ts...>::type& get(const Tuple<Ts...>& tuple) {
    return Getter<k>::get(tuple);
};

template<size_t k, typename... Ts>
constexpr typename elem_type_holder<k, Ts...>::type get(Tuple<Ts...>&& tuple){
    return Getter<k>::get(std::move(tuple));
};

// get by type
template<typename U, typename T, typename... Ts>
constexpr typename std::enable_if_t<std::is_same<U, T>::value, U&> get(Tuple<T, Ts...>& tuple){
    return tuple.tail;
};

template<typename U, typename T, typename... Ts>
constexpr typename std::enable_if_t<!std::is_same<U, T>::value, U&> get(Tuple<T, Ts...>& tuple){
    return get<U>(tuple.parent(tuple));
};


template<typename U, typename T, typename... Ts>
constexpr typename std::enable_if_t<std::is_same<U, T>::value, const U&> get(const Tuple<T, Ts...>& tuple){
    return tuple.tail;
};

template<typename U, typename T, typename... Ts>
constexpr typename std::enable_if_t<!std::is_same<U, T>::value, const U&> get(const Tuple<T, Ts...>& tuple){
    return get<U>(tuple.parent(tuple));
};


template<typename U, typename T, typename... Ts>
constexpr typename std::enable_if_t<std::is_same<U, T>::value, U> get(Tuple<T, Ts...>&& tuple){
    return std::move(tuple.tail);
};

template<typename U, typename T, typename... Ts>
constexpr typename std::enable_if_t<!std::is_same<U, T>::value, U> get(Tuple<T, Ts...>&& tuple){
    return get<U>(std::move(tuple.parent(tuple)));
};

template<typename T, typename... Ts>
constexpr auto tupleCat(const T& first, const Ts&... other){
    return tupleCat(first, tupleCat(other...));
};

template<typename... T, typename... U>
constexpr auto tupleCat(const Tuple<T...>& first, const Tuple<U...>& second){
    Tuple<T..., U...> res;
    get<0>(res) = get<0>(first);
    res.parent(res) = tupleCat(first.parent(first), second);
    return res;
};

template<typename T, typename... U>
constexpr auto tupleCat(const Tuple<T>& first, const Tuple<U...>& second){
    Tuple<T, U...> res;
    get<0>(res) = get<0>(first);
    res.parent(res) = second;
    return res;
};

template<typename... T, typename... U>
bool operator<(const Tuple<T...>& first, const Tuple<U...>& second) {
    return get<0>(first) < get<0>(second) ||
            (get<0>(first) == get<0>(second) && first.parent(first) < first.parent(first));
};

template <typename T, typename U>
bool operator<(const Tuple<T>& first, const Tuple<U>& second) {
    return get<0>(first) < get<0>(second);
};


template<typename... T, typename... U>
bool operator==(const Tuple<T...>& first, const Tuple<U...>& second) {
    return get<0>(first) == get<0>(second) && first.parent(first) < first.parent(first);
};

template <typename T, typename U>
bool operator==(const Tuple<T>& first, const Tuple<U>& second) {
    return get<0>(first) == get<0>(second);
};

template <typename T, typename U>
bool operator<=(const Tuple<T>& first, const Tuple<U>& second) {
    return first < second || first == second;
};

template <typename T, typename U>
bool operator>(const Tuple<T>& first, const Tuple<U>& second) {
    return !(first <= second);
};

template <typename T, typename U>
bool operator>=(const Tuple<T>& first, const Tuple<U>& second) {
    return !(first < second);
};