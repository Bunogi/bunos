#pragma once

#include <bustd/stddef.hpp>
#include <bustd/type_traits.hpp>

namespace kernel {
template <typename T> class AddressContainer {
  static_assert(bu::is_integral_type<T>::value);
  static_assert(sizeof(void *) <= sizeof(T),
                "T must be able to contain a void*");

public:
  AddressContainer() : m_addr(0){};
  explicit constexpr AddressContainer(T addr) : m_addr(addr) {}
  explicit constexpr AddressContainer(const AddressContainer &other) {
    *this = other.m_addr;
  }

  constexpr T get() const { return m_addr; }
  constexpr AddressContainer &operator=(T addr) {
    m_addr = addr;
    return *this;
  }

  constexpr AddressContainer &operator=(const AddressContainer &other) {
    m_addr = other.m_addr;
    return *this;
  }

  constexpr bool operator==(const AddressContainer &other) const {
    return m_addr == other.m_addr;
  }

  constexpr AddressContainer operator+(T to_add) {
    return AddressContainer<T>(to_add + m_addr);
  }
  constexpr AddressContainer &operator+=(T to_add) {
    m_addr += to_add;
    return *this;
  }

  constexpr operator bool() const { return m_addr != 0; }

  using Type = T;

  void *ptr() const {
    static_assert(
        sizeof(T) == sizeof(void *),
        "The size of the address must be equal to the size of a pointer");
    return reinterpret_cast<void *>(m_addr);
  }

protected:
  T m_addr{};
};

class PhysicalAddress : public AddressContainer<u64> {
  using AddressContainer<u64>::AddressContainer;
};

class VirtualAddress : public AddressContainer<uintptr_t> {
  using AddressContainer<uintptr_t>::AddressContainer;

public:
  explicit constexpr VirtualAddress(void *p)
      : AddressContainer(reinterpret_cast<Type>(p)) {}

  constexpr PhysicalAddress to_linked_location() const {
    return PhysicalAddress(m_addr - 0xC0000000);
  }
};

} // namespace kernel
