#pragma once

#include<type_traits>

template<typename T>
using IsNotReference = std::enable_if_t<!std::is_reference_v<T>>;