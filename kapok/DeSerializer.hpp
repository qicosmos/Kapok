#pragma once
#include "JsonUtil.hpp"
#include "traits.hpp"
#include <boost/lexical_cast.hpp>

class DeSerializer : NonCopyable
{
public:
	DeSerializer() = default;
	DeSerializer(const char* jsonText)
	{
		Parse(jsonText);
	}
	
	DeSerializer(const string& jsonText)
	{
		Parse(jsonText);
	}

	~DeSerializer()
	{
	}
	
	void Parse(const string& jsonText)
	{
		Parse(jsonText.c_str());
	}

	void Parse(const char* jsonText)
	{
		m_jsutil.Parse(jsonText);
	}

	Document&  GetDocument()
	{
		return m_jsutil.GetDocument();
	}
	
	template<typename T>
	void Deserialize(T& t, const string& key, bool has_root = true)
	{
		Deserialize(t, key.c_str(), has_root);
	}

	template<typename T>
	void Deserialize(T& t, const char* key, bool has_root = true)
	{
		Value& jsonval = GetRootValue(key, has_root);

		ReadObject(t, jsonval);
	}

	template<typename T>
	void Deserialize(T& t, bool has_root = true)
	{
		Value& jsonval = GetRootValue(nullptr, has_root);

		ReadObject(t, jsonval);
	}

	template <typename T, size_t N>
	void Deserialize(T(&p)[N], const char* key)
	{
		ReadArray<T>(p, key);
	}

	template<typename T, size_t N>
	void Deserialize(std::array<T, N>& arr, const char* key)
	{
		ReadArray<T>(arr, key);
	}

private:
    Value& GetRootValue(const char* key, bool has_root)
	{
		Document& doc = m_jsutil.GetDocument();
		if (!has_root)
			return doc;

		if (key == nullptr)
		{
			auto it = doc.MemberBegin();
			return (Value&)it->value;
		}
			
		if (!doc.HasMember(key))
			throw std::invalid_argument("the key is not exist");

		return doc[key];
	}
	
	template<typename value_type, typename T>
	void ReadArray(T& t, const char* key)
	{
		Value& jsonval = GetRootValue(key);

		ReadArray<value_type>(t, jsonval);
	}

	template<typename value_type, typename T>
	void ReadArray(T& t, Value& jsonval)
	{
		size_t sz = jsonval.Size();
		for (SizeType i = 0; i < sz; i++)
		{
			value_type value;
			ReadValue(value, jsonval[i]);
			t[i] = value;
		}
	}

	template<typename T>
	typename std::enable_if<is_tuple<T>::value>::type ReadObject(T& t, Value& val)
	{
		ReadTuple(std::forward<T>(t), val);
	}

	template<typename T>
	typename std::enable_if<is_user_class<T>::value>::type ReadObject(T& t, Value& val)
	{
		ReadTuple(t.Meta(), val);
	}

	template<typename Tuple>
	void ReadTuple(Tuple&& tp, Value& val)
	{
		const int size = std::tuple_size<Tuple>::value;
		
		for (SizeType j = 0; j < size; j++)
		{
			SetValueByIndex(j, tp, val);
		}
	}

	template<size_t k=0, typename Tuple>
	inline typename std::enable_if < (k == std::tuple_size<Tuple>::value)>::type SetValueByIndex(size_t, Tuple&, Value&)
	{
		throw std::invalid_argument("arg index out of range");
	}

	template<size_t k = 0, typename Tuple>
	inline typename std::enable_if < (k < std::tuple_size<Tuple>::value)>::type SetValueByIndex(size_t index, Tuple& tp, Value& val)
	{
		if (k == index)
		{
			ReadValue<k>(tp, val);
		}
		else
		{
			SetValueByIndex<k + 1>(index, tp, val);
		}
	}

	template<typename T>
	typename std::enable_if<is_singlevalue_container<T>::value || is_container_adapter<T>::value>::type ReadObject(T&& t, Value& v)
	{
		using U = typename std::decay<T>::type;

		size_t sz = v.Size();
		for (SizeType i = 0; i < sz; i++)
		{
			typename U::value_type value;
			ReadObject(value, v[i]);
			push(t, value, i);
		}
	}

	template<typename T>
	typename std::enable_if<std::is_array<T>::value||is_std_array<T>::value>::type ReadObject(T&& t, Value& v)
	{
		using U = typename std::decay<T>::type;

		size_t sz = v.Size();
		for (SizeType i = 0; i < sz; i++)
		{
			typename U::value_type value;
			ReadObject(value, v[i]);
			t[i] = value;
		}
	}

	template<typename T, typename value_type>
	typename std::enable_if<is_singlevalue_container<T>::value&&!is_set<T>::value&&!is_multiset<T>::value&&!is_unordered_set<T>::value>::type push(T& t, value_type& v, std::size_t index)
	{
		t.push_back(v);
	}

	template<typename T, typename value_type>
	typename std::enable_if<is_set<T>::value||is_multiset<T>::value|| is_unordered_set<T>::value>::type push(T& t, value_type& v, std::size_t index)
	{
		t.insert(v);
	}

	template<typename T, typename value_type>
	typename std::enable_if<is_std_array<T>::value || std::is_array<T>::value>::type push(T& t, value_type& v, std::size_t index)
	{
		t[index] = v;
	}

	template<typename T, typename value_type>
	typename std::enable_if<is_container_adapter<T>::value>::type push(T& t, value_type& v, std::size_t index)
	{
		t.push(v);
	}

	template<typename T>
	typename std::enable_if<is_map_container<T>::value>::type ReadObject(T&& t, Value& v)
	{
		using U = typename std::decay<T>::type;

		for (rapidjson::Value::ConstMemberIterator it = v.MemberBegin(); it != v.MemberEnd(); it++)
		{
			using key_type = typename U::key_type;
			using val_type = typename U::value_type::second_type;
			key_type key;
			val_type value;

			key = boost::lexical_cast<key_type>(it->name.GetString());

			ReadObject(value, (Value&)it->value); 

			t.emplace(key, value);
		}
	}

	template<typename T>
	typename std::enable_if<is_basic_type<T>::value>::type ReadObject(T&& t, Value& v)
	{
		m_jsutil.ReadValue(std::forward<T>(t), v);
	}

	template<size_t N=0, typename T>
	typename std::enable_if<is_user_class<T>::value>::type ReadValue(T&& t, Value& val)
	{
		//Value& p = val[t.first.c_str()];
		ReadObject(t, val);
	}

	template<size_t N = 0, typename T>
	typename std::enable_if<is_pair<T>::value>::type ReadObject(T&& t, Value& val)
	{
		ReadObject(t.second, val);
	}

	template<size_t N = 0, typename T>
	typename std::enable_if<is_basic_type<T>::value>::type ReadValue(T&& t, Value& val)
	{
		m_jsutil.ReadValue(std::forward<T>(t), val);
	}

	template<size_t N = 0, typename T>
	typename std::enable_if<is_tuple<T>::value>::type ReadValue(T&& t, Value& val)
	{
		if (val.IsArray())
		{
			ReadObject(std::get<N>(t), val[N]);
		}
		else
		{
			auto it = val.MemberBegin() + N;
			ReadObject(std::get<N>(t), (Value&)(it->value));
		}
	}

private:
	JsonUtil m_jsutil;
};

