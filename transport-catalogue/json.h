#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

using Value =
    std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

class ParsingError : public std::runtime_error {
 public:
  using runtime_error::runtime_error;
};

class Node {
 public:
   Node();


  explicit Node(Value value);

  Node(const Array& arr);
  Node(const Dict& map);
  Node(int value);
  Node(std::string value);
  Node(double value);
  Node(bool value);
  Node(std::nullptr_t value);

  template<typename T> const T&
  GetNodeValue() const {
    if (const auto pval = std::get_if<T>(&m_value)) {
      return *pval;
    } else {
      throw std::logic_error("Not array");
    }
  }
  const Array& AsArray() const;
  const Dict& AsMap() const;
  int AsInt() const;
  const std::string& AsString() const;
  double AsDouble() const;
  double AsBool() const;

  const Value& GetValue() const { return m_value; }

  bool IsInt() const;
  bool IsDouble() const;
  bool IsPureDouble() const;
  bool IsString() const;
  bool IsNull() const;
  bool IsArray() const;
  bool IsMap() const;
  bool IsBool() const;
  
  
 private:
  Value m_value;
};

inline bool operator==(const Node &lhs, const Node &rhs) {
  return lhs.GetValue() == rhs.GetValue();
}
inline bool operator!=(const Node &lhs, const Node &rhs) {
  return lhs.GetValue() != rhs.GetValue();
}




// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
  std::ostream& out;
  int indent_step = 4;
  int indent = 0;

  void PrintIndent() const {
    for (int i = 0; i < indent; ++i) {
      out.put(' ');
    }
  }

  // Возвращает новый контекст вывода с увеличенным смещением
  PrintContext Indented() const {
    return {out, indent_step, indent_step + indent};
  }
};

void PrintValue(std::nullptr_t, std::ostream& out);
void PrintValue(Array arr, std::ostream& out);
void PrintValue(Dict dict, std::ostream& out);
void PrintValue(bool val, std::ostream &out);
void PrintValue(int numeric_value, std::ostream& out);
void PrintValue(double double_value, std::ostream& out);
void PrintValue(std::string string_value, std::ostream& out);


template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintNode(const Node& node, std::ostream& out);

using Number = std::variant<int, double>;


class Document {
 public:
  explicit Document(Node root);

  const Node& GetRoot() const;

 private:
  Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);


inline bool operator==(const Document &lhs, const Document &rhs) {
  return lhs.GetRoot().GetValue() == rhs.GetRoot().GetValue();
}
inline bool operator!=(const Document &lhs, const Document &rhs) {
  return lhs.GetRoot().GetValue() != rhs.GetRoot().GetValue();
}
}  // namespace json