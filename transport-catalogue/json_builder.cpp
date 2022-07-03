#include "json_builder.h"


namespace json {

	using namespace std;

	Builder::Builder() :m_prev_command(Command::Undefined), m_general_context(*this), m_dict_context(*this), m_key_context(*this), m_key_value_context(*this),
		m_array_context(*this), m_value_array_context(*this) 
	{}

	DictItemContext& Builder::StartDict()
	{
		if (m_nodes_stack.empty() && Command::Undefined != m_prev_command) {
			throw logic_error("Adds after end");
		}
		if (m_nodes_stack.empty()) {
			m_root = Dict();
			m_nodes_stack.push_back(&m_root);
			return m_dict_context;
		}

		Node* parent = m_nodes_stack.back();

		if (parent->IsArray()) {
			parent->AsArray().push_back(Dict());
			m_nodes_stack.push_back(&parent->AsArray().back());
		}
		else if (Command::Key == m_prev_command) {
			m_nodes_stack.back()->GetValue() = Dict();
		}
		else {
			throw logic_error("StartDict");
		}
		m_prev_command = Command::StartDict;
		return m_dict_context;
	}

	Builder& Builder::EndDict() {
		if (m_nodes_stack.empty())
		{
			throw logic_error("Empty");
		}
		if (!m_nodes_stack.back()->IsDict()) {
			throw logic_error("Not dict");
		}
		m_nodes_stack.pop_back();
		m_prev_command = Command::EndDict;
		return *this;
	}

	ArrayItemContext& Builder::StartArray()
	{
		if (m_nodes_stack.empty() && Command::Undefined != m_prev_command) {
			throw logic_error("Adds after end");
		}
		if (m_nodes_stack.empty()) {
			m_root = json::Array();
			m_nodes_stack.push_back(&m_root);
			return m_array_context;
		}

		Node* parent = m_nodes_stack.back();

		if (parent->IsArray()) {
			parent->AsArray().push_back(Array());
			m_nodes_stack.push_back(&parent->AsArray().back());
		}
		else if (Command::Key == m_prev_command) {
			m_nodes_stack.back()->GetValue() = Array();
		}
		else {
			throw std::logic_error("StartArray");
		}
		m_prev_command = Command::StartArray;
		return m_array_context;
	}

	Builder& Builder::EndArray()
	{
		if (m_nodes_stack.empty())
		{
			throw logic_error("Empty");
		}

		if (!m_nodes_stack.back()->IsArray()) {
			throw logic_error("Not array");
		}
		m_nodes_stack.pop_back();
		m_prev_command = Command::EndArray;
		return *this;

	}

	KeyItemContext& Builder::Key(std::string&& key)
	{

		if (m_nodes_stack.empty() || !m_nodes_stack.back()->IsDict() || Command::Key == m_prev_command)
		{
			throw logic_error("NOt valid key");
		}
		auto& d = m_nodes_stack.back()->AsDict();
		m_nodes_stack.push_back(&d[move(key)]);
		m_prev_command = Command::Key;
		return m_key_context;
	}

	Builder& Builder::Value(Node::Value&& value)
	{
		if (m_nodes_stack.empty() && Command::Undefined != m_prev_command) {
			throw logic_error("Adds after end");
		}
		Node n;
		n.GetValue() = move(value);
		bool arr_or_dict = n.IsArray() || n.IsDict();


		if (m_nodes_stack.empty()) {
			m_root = move(n);
			//if (arr_or_dict) {
			//	m_nodes_stack.push_back(&m_root);
			//}
		}
		else if (Command::Key == m_prev_command) {
			m_nodes_stack.back()->GetValue() = move(n.GetValue());
			if (!arr_or_dict) {
				m_nodes_stack.pop_back();
			}
		}
		else if (m_nodes_stack.back()->IsArray()) {
			m_nodes_stack.back()->AsArray().push_back(move(n));
			if (arr_or_dict) {
				m_nodes_stack.push_back(&m_nodes_stack.back()->AsArray().back());
			}
		}
		else
		{
			throw logic_error("Not valid value");
		}
		m_prev_command = Command::Value;
		return *this;
	}

	json::Node& Builder::Build()
	{
		if (!m_nodes_stack.empty() || Command::Undefined == m_prev_command) {
			throw logic_error("Empty root");
		}
		return m_root;
	}

	Context& Builder::GetContext()
	{
		return m_general_context;
	}

	DictItemContext& Builder::GetDictItemContext()
	{
		return m_dict_context;
	}

	KeyItemContext& Builder::GetKeyItemContext()
	{
		return  m_key_context;
	}

	KeyValueItemContext& Builder::GetKeyValueItemContext()
	{
		return m_key_value_context;
	}

	ArrayItemContext& Builder::GetArrayItemContext()
	{
		return  m_array_context;
	}

	ValueArrayItemContext& Builder::GetValueArrayItemContext()
	{
		return m_value_array_context;
	}

	Context::Context(Builder& builder) :m_builder(builder)
	{
	}
	DictItemContext& Context::StartDict()
	{
		return m_builder.StartDict();
	}
	Builder& Context::EndDict()
	{
		return m_builder.EndDict();
	}
	ArrayItemContext& Context::StartArray()
	{
		return m_builder.StartArray();
	}
	Builder& Context::EndArray()
	{
		return m_builder.EndArray();
	}
	KeyItemContext& Context::Key(std::string&& key)
	{
		m_builder.Key(move(key));
		return m_builder.GetKeyItemContext();
	}
	//Context& Context::Value(Node::Value&& value)
	//{
	//	m_builder.Value(move(value));
	//	return *this;
	//}
	DictItemContext::DictItemContext(Builder& builder) :Context(builder) {
	}
	KeyItemContext& DictItemContext::Key(std::string&& key)
	{
		m_builder.Key(move(key));
		return m_builder.GetKeyItemContext();
	}
	Builder& DictItemContext::EndDict()
	{
		return m_builder.EndDict();
	}
	DictItemContext& DictItemContext::StartDict()
	{
		return Context::StartDict();
	}
	ArrayItemContext& DictItemContext::StartArray()
	{
		return m_builder.StartArray();
	}
	Builder& DictItemContext::EndArray()
	{
		return Context::EndArray();
	}
	Context& DictItemContext::Value(Node::Value&& value)
	{
		m_builder.Value(move(value));
		return m_builder.GetContext();
	}
	KeyItemContext::KeyItemContext(Builder& builder):Context(builder)
	{
	}
	KeyValueItemContext& KeyItemContext::Value(Node::Value&& value)
	{
		m_builder.Value(move(value));
		return m_builder.GetKeyValueItemContext();
	}

	DictItemContext& KeyItemContext::StartDict()
	{
		m_builder.StartDict();
		return m_builder.GetDictItemContext();
	}
	ArrayItemContext& KeyItemContext::StartArray()
	{
		return m_builder.StartArray();
	}

	Builder& KeyItemContext::EndDict()
	{
		return Context::EndDict();
	}

	Builder& KeyItemContext::EndArray()
	{
		return Context::EndArray();
	}

	KeyItemContext& KeyItemContext::Key(std::string&& key)
	{
		return Context::Key(move(key));
	}

	KeyValueItemContext::KeyValueItemContext(Builder& builder):Context(builder)
	{
	}
	KeyItemContext& KeyValueItemContext::Key(std::string&& key)
	{
		return m_builder.Key(move(key));
	}
	Builder& KeyValueItemContext::EndDict()
	{
		return m_builder.EndDict();
	}
	DictItemContext& KeyValueItemContext::StartDict()
	{
		return Context::StartDict();
	}
	ArrayItemContext& KeyValueItemContext::StartArray()
	{
		return Context::StartArray();
	}
	Builder& KeyValueItemContext::EndArray()
	{
		return m_builder.EndArray();
	}
	DictItemContext& KeyValueItemContext::Value(Node::Value&& value)
	{
		m_builder.Value(move(value));
		return m_builder.GetDictItemContext();
	}
	ArrayItemContext::ArrayItemContext(Builder& builder):Context(builder)
	{
	}
	ValueArrayItemContext& ArrayItemContext::Value(Node::Value&& value)
	{
		m_builder.Value(move(value));
		return m_builder.GetValueArrayItemContext();
	}
	DictItemContext& ArrayItemContext::StartDict()
	{
		m_builder.StartDict();
		return m_builder.GetDictItemContext();
	}
	ArrayItemContext& ArrayItemContext::StartArray()
	{
		return m_builder.StartArray();
	}
	Builder& ArrayItemContext::EndArray()
	{
		return Context::EndArray();
	}
	Builder& ArrayItemContext::EndDict()
	{
		return Context::EndDict();
	}
	KeyItemContext& ArrayItemContext::Key(std::string&& key)
	{
		return Context::Key(move(key));
	}
	ValueArrayItemContext::ValueArrayItemContext(Builder& builder):Context(builder)
	{
	}
	ValueArrayItemContext& ValueArrayItemContext::Value(Node::Value&& value)
	{
		m_builder.Value(move(value));
		return m_builder.GetValueArrayItemContext();
	}
	DictItemContext& ValueArrayItemContext::StartDict()
	{
		return m_builder.StartDict();
	}
	ArrayItemContext& ValueArrayItemContext::StartArray()
	{
		return m_builder.StartArray();
	}
	Builder& ValueArrayItemContext::EndArray()
	{
		return m_builder.EndArray();
	}
	Builder& ValueArrayItemContext::EndDict()
	{
		return Context::EndDict();
	}
	KeyItemContext& ValueArrayItemContext::Key(std::string&& key)
	{
		return Context::Key(move(key));
	}
}

