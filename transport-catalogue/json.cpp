#include "json.h"

#include <stdexcept>
#include <sstream>

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
  char c1;
  if (!(input >> c1)) {
    throw json::ParsingError("Not valid array");
  }
  input.putback(c1);
    
  Array result;
  char c;
  for (; input >> c && c != ']';) {
    if (c != ',') {
      input.putback(c);
    }
    result.push_back(LoadNode(input));
  }

  return Node(move(result));
}


Number LoadNumber(std::istream &input) {
  using namespace std::literals;

  std::string parsed_num;

  // Считывает в parsed_num очередной символ из input
  auto read_char = [&parsed_num, &input] {
    parsed_num += static_cast<char>(input.get());
    if (!input) {
      throw ParsingError("Failed to read number from stream"s);
    }
  };

  // Считывает одну или более цифр в parsed_num из input
  auto read_digits = [&input, read_char] {
    if (!std::isdigit(input.peek())) {
      throw ParsingError("A digit is expected"s);
    }
    while (std::isdigit(input.peek())) {
      read_char();
    }
  };

  if (input.peek() == '-') {
    read_char();
  }
  // Парсим целую часть числа
  if (input.peek() == '0') {
    read_char();
    // После 0 в JSON не могут идти другие цифры
  } else {
    read_digits();
  }

  bool is_int = true;
  // Парсим дробную часть числа
  if (input.peek() == '.') {
    read_char();
    read_digits();
    is_int = false;
  }

  // Парсим экспоненциальную часть числа
  if (int ch = input.peek(); ch == 'e' || ch == 'E') {
    read_char();
    if (ch = input.peek(); ch == '+' || ch == '-') {
      read_char();
    }
    read_digits();
    is_int = false;
  }

  try {
    if (is_int) {
      // Сначала пробуем преобразовать строку в int
      try {
        return std::stoi(parsed_num);
      } catch (...) {
        // В случае неудачи, например, при переполнении,
        // код ниже попробует преобразовать строку в double
      }
    }
    return std::stod(parsed_num);
  } catch (...) {
    throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
  }
}


Node LoadString(istream& input) {
  using namespace std::literals;

  auto it = std::istreambuf_iterator<char>(input);
  auto end = std::istreambuf_iterator<char>();
  std::string s;
  while (true) {
    if (it == end) {
      // Поток закончился до того, как встретили закрывающую кавычку?
      throw ParsingError("String parsing error");
    }
    const char ch = *it;
    if (ch == '"') {
      // Встретили закрывающую кавычку
      ++it;
      break;
    } else if (ch == '\\') {
      // Встретили начало escape-последовательности
      ++it;
      if (it == end) {
        // Поток завершился сразу после символа обратной косой черты
        throw ParsingError("String parsing error");
      }
      const char escaped_char = *(it);
      // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
      switch (escaped_char) {
      case 'n':
        s.push_back('\n');
        break;
      case 't':
        s.push_back('\t');
        break;
      case 'r':
        s.push_back('\r');
        break;
      case '"':
        s.push_back('"');
        break;
      case '\\':
        s.push_back('\\');
        break;
      default:
        // Встретили неизвестную escape-последовательность
        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
      }
    } else if (ch == '\n' || ch == '\r') {
      // Строковый литерал внутри- JSON не может прерываться символами \r или \n
      throw ParsingError("Unexpected end of line"s);
    } else {
      // Просто считываем очередной символ и помещаем его в результирующую
      // строку
      s.push_back(ch);
    }
    ++it;
  }

  return s;
}

Node LoadDict(istream& input) {
  Dict result;
  char c1;
  if (!(input >> c1)) {
    throw json::ParsingError("Not valid dictionary");
  }
  input.putback(c1);
  
  for (char c; input >> c && c != '}';) {
    if (c == ',') {
      input >> c;
    }

    string key = LoadString(input).AsString();
    input >> c;
    result.insert({move(key), LoadNode(input)});
  }

  return Node(move(result));
}

Node LoadNode(istream& input) {
  char c;
  input >> c;
  if (c == 'n') {
    string s;
    s += c;
    for (int i = 0; i < 3; ++i) {
      if (input >> c) {
        s += c;
      }
    }
    if (s != "null") {
      throw ParsingError("Error");
    } else {
      return Node(nullptr);
    }
  } else if (c == 't') {
    string s;
    s += c;
    for (int i = 0; i < 3; ++i) {
      input >> c;
      s += c;
    }
    if (s != "true") {
      throw ParsingError("Error");
    } else {
      return Node(true);
    }
  }else if (c == 'f') {
        string s;
        s += c;
        for (int i = 0; i < 4; ++i) {
          if (input >> c) {
            s += c;
          }
        }
        if (s != "false") {
          throw ParsingError("Error");
        } else {
          return Node(false);
        }
  }else if (c == '[') {
    return LoadArray(input);
  } else if (c == '{') {
    return LoadDict(input);
  } else if (c == '"') {
    return LoadString(input);
  } else {
    input.putback(c);
    auto number= LoadNumber(input);
    if (std::holds_alternative<int>(number)) {
      return Node(std::get<int>(number));
    }
    return Node(std::get<double>(number));
  }
}

}  // namespace

Node::Node() {}

Node::Node(Value value) : m_value(move(value)) {}

Node::Node(const Array &arr) { m_value.emplace<Array>(arr); }

Node::Node(const Dict &map) { m_value.emplace<Dict>(map); }

Node::Node(int value) { m_value.emplace<int>(value); }

Node::Node(std::string value) { 
    m_value.emplace<std::string>(value); 
}

Node::Node(double value) {
    m_value.emplace<double>(value);
}

Node::Node(bool value) { m_value.emplace<bool>(value); }

Node::Node(std::nullptr_t value) { m_value.emplace<std::nullptr_t>(value); }

const Array& Node::AsArray() const { return GetNodeValue<Array>(); }

const Dict& Node::AsMap() const { return GetNodeValue<Dict>(); }

int Node::AsInt() const { return GetNodeValue<int>(); }

const string &Node::AsString() const { return GetNodeValue<std::string>(); }

double Node::AsDouble() const {
    if (IsInt()) {
      return static_cast<double>(GetNodeValue<int>());
  }
  return GetNodeValue<double>();
}

double Node::AsBool() const { return GetNodeValue<bool>(); }

bool Node::IsInt() const { return std::holds_alternative<int>(m_value); }

bool Node::IsDouble() const { return IsInt() || IsPureDouble(); }

bool Node::IsPureDouble() const { return std::holds_alternative<double>(m_value); }

bool Node::IsString() const { return std::holds_alternative<std::string>(m_value); }

bool Node::IsNull() const { return std::holds_alternative<std::nullptr_t>(m_value); }

bool Node::IsArray() const { return std::holds_alternative<Array>(m_value); }

bool Node::IsMap() const { return std::holds_alternative<Dict>(m_value); }

bool Node::IsBool() const { return std::holds_alternative<bool>(m_value); }

Document::Document(Node root) : root_(move(root)) {}

const Node& Document::GetRoot() const { return root_; }

Document Load(istream& input) { return Document{LoadNode(input)}; }

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);

}

void PrintValue(std::nullptr_t, std::ostream& out) { out << "null"sv; }

void PrintValue(Array arr, std::ostream& out) { 
    out << '[';
  for (size_t i = 0; i < arr.size(); ++i) {
      PrintNode(arr[i], out);
    if (i != arr.size() - 1) {
        out << ',';
    }
  }
  out << ']';
}

void PrintValue(Dict dict, std::ostream& out)
{
  out << '{';
  
  for (auto it = dict.begin(); it != dict.end();) {
    PrintValue(it->first, out);
    out << ":";
    /*out << '\"' << it->first << "\": ";*/
    PrintNode(it->second, out);
    if (++it != dict.end()) {
      out << ',';
    }
  }
  out << '}';
}

void PrintValue(bool val, std::ostream &out) { 
    out << val; 
}

void PrintValue(int numeric_value, std::ostream& out)
{ out << numeric_value; }
void PrintValue(double numeric_value, std::ostream &out) { out << numeric_value; }

void replaceAll(std::string &str, const std::string &from,
                const std::string &to) {
  if (from.empty())
    return;
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing
                              // 'x' with 'yx'
  }
}
void PrintValue(std::string string_value, std::ostream& out)
{
  replaceAll(string_value, "\\", "\\\\");
  replaceAll(string_value, "\"", "\\\"");
  replaceAll(string_value, "\r", "\\r");
  replaceAll(string_value, "\n", "\\n");
 out << "\"" << string_value << "\"";
}

void PrintNode(const Node& node, std::ostream& out)
{
  out << std::boolalpha;
    std::visit([&out](const auto& value) { PrintValue(value, out); },
        node.GetValue());
}

}  // namespace json