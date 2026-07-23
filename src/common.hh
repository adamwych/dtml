#pragma once

#include <cstdlib>
#include <cstring>
#include <map>
#include <optional>
#include <stack>
#include <stdio.h>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

template <typename K, typename V> using Map = std::map<K, V>;
template <typename E> using Vector = std::vector<E>;
template <typename E> using Stack = std::stack<E>;
template <typename T> using Option = std::optional<T>;
using String = std::string;
using StringView = std::string_view;

#define Nullopt std::nullopt