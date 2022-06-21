#pragma once

#include "json.h"


namespace json {

	class Builder;
	class KeyItemContext;
	class KeyValueItemContext;
	class DictItemContext;
	class ArrayItemContext;
	class ValueArrayItemContext;
	 
	class Context {
	public:
		Context(Builder& builder);
        virtual DictItemContext& StartDict();
		virtual Builder& EndDict();

		virtual ArrayItemContext& StartArray();
		virtual Builder& EndArray();

		virtual KeyItemContext& Key(std::string&& key);
		//virtual Context& Value(Node::Value&& value);
	protected:
		Builder& m_builder;
	};
	
	class DictItemContext :public Context{
	public:
		DictItemContext(Builder& builder);

		KeyItemContext& Key(std::string&& key) override;
		Builder& EndDict() override;

	private:
		DictItemContext& StartDict() override;
		
		ArrayItemContext& StartArray() override;
		Builder& EndArray() override;

		Context& Value(Node::Value&& value);
	};

	class ArrayItemContext :public Context {
	public:
		ArrayItemContext(Builder& builder);
		ValueArrayItemContext& Value(Node::Value&& value);
		DictItemContext& StartDict() override;
		ArrayItemContext& StartArray() override;
		Builder& EndArray() override;

	private:
		Builder& EndDict() override;
		KeyItemContext& Key(std::string&& key) override;
	};

	class KeyItemContext :public Context {
	public:
		KeyItemContext(Builder& builder);
		KeyValueItemContext& Value(Node::Value&& value);
		DictItemContext& StartDict() override;
		ArrayItemContext& StartArray() override;

	private:
		Builder& EndDict() override;
		Builder& EndArray() override;
		KeyItemContext& Key(std::string&& key) override;
	};

	class KeyValueItemContext :public Context {
	public:
		KeyValueItemContext(Builder& builder);
		KeyItemContext& Key(std::string&& key) override;
		Builder& EndDict() override;

	private:
		DictItemContext& StartDict() override;
		ArrayItemContext& StartArray() override;
		Builder& EndArray() override;
		DictItemContext& Value(Node::Value&& value);
	};

	class ValueArrayItemContext :public Context {
	public:
		ValueArrayItemContext(Builder& builder);
		ValueArrayItemContext& Value(Node::Value&& value);
		DictItemContext& StartDict() override;
		ArrayItemContext& StartArray() override;
		Builder& EndArray() override;

	private:
		Builder& EndDict() override;
		KeyItemContext& Key(std::string&& key) override;
	};


	class Builder {
	public:
		Builder();
		DictItemContext& StartDict();
		Builder& EndDict();

		ArrayItemContext& StartArray();
		Builder& EndArray();

		KeyItemContext& Key(std::string&& key);
		Builder& Value(Node::Value&& value);

		json::Node& Build();

		Context& GetContext();
		DictItemContext& GetDictItemContext();
		KeyItemContext& GetKeyItemContext();
		KeyValueItemContext& GetKeyValueItemContext();
		ArrayItemContext& GetArrayItemContext();
		ValueArrayItemContext& GetValueArrayItemContext();

	private:
		Node m_root;
		std::vector<Node*> m_nodes_stack;
		enum class Command {
			Undefined = 0,
			StartArray = 1,
			StartDict = 2,
			Key = 3,
			EndDict = 4,
			EndArray = 5,
			Value = 6
		};
		Command m_prev_command;

	private:
		Context m_general_context;
		DictItemContext m_dict_context;
		KeyItemContext m_key_context;
		KeyValueItemContext m_key_value_context;
		ArrayItemContext m_array_context;
		ValueArrayItemContext m_value_array_context;
	};
}

