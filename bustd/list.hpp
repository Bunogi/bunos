#pragma once

#include <bustd/assert.hpp>
#include <bustd/function.hpp>
#include <bustd/stddef.hpp>

namespace bu {
template <typename T> class List {
  struct Node {
    Node() = delete;
    Node(T &&v) : val(forward(v)) {}
    Node(const T &v) : val(v) {}

    Node *prev{nullptr};
    Node *next{nullptr};
    T val;
  };

public:
  List() = default;

  List(const List &other) { *this = other; }

  auto operator=(const List &other) -> List & {
    clear();

    const auto *node = other.m_head;
    while (node) {
      append_back(node->val);
      node = node->next;
    }
    return *this;
  }

  List(List &&other) { *this = move(other); }

  auto operator=(List &&other) -> List & {
    m_size = other.m_size;
    m_tail = other.m_tail;
    m_head = other.m_head;

    other.m_tail = other.m_head = nullptr;
    other.m_size = 0;
  }
  ~List() { clear(); }

  void append_back(const T &val) {
    auto *new_node = new Node(val);
    ASSERT_NE(new_node, nullptr);
    insert_back(new_node);
  }

  // FIXME: this is not really emplace...
  void emplace_back(T &&val) {
    auto *new_node = new Node(forward(val));
    ASSERT_NE(new_node, nullptr);
    insert_back(new_node);
  }

  void append_front(const T &val) {
    auto *new_node = new Node(val);
    ASSERT_NE(new_node, nullptr);
    insert_front(new_node);
  }

  void emplace_front(T &&val) {
    auto *new_node = new Node(forward(val));
    ASSERT_NE(new_node, nullptr);
    insert_front(new_node);
  }

  void take_back(List<T> &other, usize index) {
    Node *const stolen = other.take_node(index);
    insert_back(stolen);
  }

  auto get(usize index) const -> const T & { return at_index(index)->val; }
  auto get(usize index) -> T & { return at_index(index)->val; }

  auto front() -> T & {
    ASSERT(m_head);
    return m_head->val;
  }

  auto front() const -> const T & {
    ASSERT(m_head);
    return m_head->val;
  }

  auto back() -> T & {
    ASSERT(m_tail);
    return m_tail->val;
  }

  auto back() const -> const T & {
    ASSERT(m_tail);
    return m_tail->val;
  }

  class Iterator {
  public:
    using Value = T;

    auto operator++() const -> Iterator {
      m_this = m_reverse ? m_this->prev : m_this->next;
      return *this;
    }
    auto operator++(int) const -> Iterator {
      Iterator tmp(*this);
      operator++();
      return tmp;
    }
    auto operator*() -> T & { return m_this->val; }
    auto operator*() const -> const T & { return m_this->val; }
    auto operator==(const Iterator &other) const -> bool {
      return m_this == other.m_this;
    }
    auto operator!=(const Iterator &other) const -> bool {
      return !(*this == other);
    }

  private:
    friend class List<T>;
    Iterator(Node *start, bool reverse) : m_this(start), m_reverse(reverse) {}
    mutable Node *m_this;
    bool m_reverse;
  };

  auto begin() -> Iterator { return Iterator(m_head, false); }

  auto rbegin() -> Iterator { return Iterator(m_tail, true); }

  auto end() -> Iterator { return Iterator(nullptr, false); }
  auto rend() -> Iterator { return Iterator(nullptr, false); }

  auto take_node(usize index) -> Node * {
    ASSERT(index < m_size);
    Node *const to_delete = at_index(index);
    if (to_delete->prev) {
      to_delete->prev->next = to_delete->next;
    }
    if (to_delete->next) {
      to_delete->next->prev = to_delete->prev;
    }

    if (to_delete == m_head) {
      m_head = to_delete->next;
    }
    if (to_delete == m_tail) {
      m_tail = to_delete->next;
    }

    m_size--;
    return to_delete;
  }

  void remove(usize index) { delete take_node(index); }

  void clear() {
    auto *node = m_head;
    while (node != nullptr) {
      auto *next = node->next;
      delete node;
      node = next;
    }
    m_head = m_tail = nullptr;
    m_size = 0;
  }

  auto len() const -> usize { return m_size; }

  void remove_if(bu::Function<bool(const T &)> f) {
    auto *this_node = m_head;
    while (this_node) {
      if (f(this_node->val)) {
        if (this_node->prev) {
          this_node->prev->next = this_node->next;
        }
        if (this_node->next) {
          this_node->next->prev = this_node->prev;
        }
        Node *next = this_node->next;
        delete this_node;
        this_node = next;
        m_size--;
      } else {
        this_node = this_node->next;
      }
    }
  }

private:
  auto at_index(usize index) -> Node * {
    ASSERT(index < m_size);
    Node *node = m_head;
    // FIXME: this can be made faster by going from the back->front if index >
    // m_size / 2
    for (usize i = 0; i < index; i++) {
      node = node->next;
    }
    return node;
  }

  void insert_back(Node *new_node) {
    ASSERT(new_node);
    if (m_size == 0) {
      m_head = m_tail = new_node;
    } else {
      auto *prev_tail = m_tail;
      ASSERT(prev_tail);
      prev_tail->next = new_node;
      new_node->prev = prev_tail;
      m_tail = new_node;
    }
    m_size++;
  }

  void insert_front(Node *new_node) {
    ASSERT(new_node);
    if (m_size == 0) {
      m_head = m_tail = new_node;
    } else {
      auto *prev_head = m_head;
      prev_head->prev = new_node;
      new_node->next = prev_head;
      m_head = new_node;
    }
    m_size++;
  }

  Node *m_head{nullptr};
  Node *m_tail{nullptr};
  usize m_size{0};
};
} // namespace bu
