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

  constexpr auto get() const -> T { return m_addr; }
  constexpr auto operator=(T addr) -> AddressContainer & {
    m_addr = addr;
    return *this;
  }

  constexpr auto
  operator=(const AddressContainer &other) -> AddressContainer & {
    m_addr = other.m_addr;
    return *this;
  }

  constexpr auto operator==(const AddressContainer &other) const -> bool {
    return m_addr == other.m_addr;
  }

  constexpr auto operator+(T to_add) -> AddressContainer {
    return AddressContainer<T>(to_add + m_addr);
  }
  constexpr auto operator+=(T to_add) -> AddressContainer & {
    m_addr += to_add;
    return *this;
  }

  constexpr operator bool() const { return m_addr != 0; }

  using Type = T;

  constexpr auto ptr() const -> void * {
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

  constexpr auto to_linked_location() const -> PhysicalAddress {
    return PhysicalAddress(m_addr - 0xC0000000);
  }
};

} // namespace kernel
