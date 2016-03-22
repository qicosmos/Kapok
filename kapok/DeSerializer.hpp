#pragma once
#include "JsonUtil.hpp"
#include "traits.hpp"

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
	void Deserialize(T& t, const string& key)
	{
		Deserialize(t, key.c_str());
	}

	template<typename T>
	void Deserialize(T& t, const char* key)
	{
		Value& jsonval = GetRootValue(key);

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
        Value& GetRootValue(const char* key)
	{
		Document& doc = m_jsutil.GetDocument();
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
		Value& arr = jsonval[size_t(0)];
		size_t sz = arr.Size();
		for (size_t i = 0; i < sz; i++)
		{
			value_type value;
			ReadValue(value, arr[i], i);
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
		if (val.Size() != std::tuple_size<Tuple>::value)
			throw std::logic_error("wrong object");

		for (size_t j = 0; j < val.Size(); j++)
		{
			SetValueByIndex(j, tp, val[rapidjson::SizeType(j)]);
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
			ReadValue(std::get<k>(tp), val, k);
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
		Value& arr = v[size_t(0)];
		size_t sz = arr.Size();
		for (size_t i = 0; i < sz; i++)
		{
			typename U::value_type value;
			ReadValue(value, arr[i], i);
			push(t, value);
		}
	}

	template<typename T, typename value_type>
	typename std::enable_if<is_singlevalue_container<T>::value>::type push(T& t, value_type& v)
	{
		t.push_back(v);
	}

	template<typename T, typename value_type>
	typename std::enable_if<is_container_adapter<T>::value>::type push(T& t, value_type& v)
	{
		t.push(v);
	}

	template<typename T>
	typename std::enable_if<is_map_container<T>::value>::type ReadObject(T&& t, Value& v)
	{
		using U = typename std::decay<T>::type;

		size_t sz = v.Size();
		for (size_t i = 0; i < sz; i++)
		{
			Value& element = v[i];

			typename U::key_type key;
			typename U::value_type::second_type value;
			ReadValue(key, element["0"], i); //key
			ReadObject(value, element["1"]); //value

			t.emplace(key, value);
		}
	}

	template<typename T>
	typename std::enable_if<is_basic_type<T>::value>::type ReadObject(T&& t, Value& v)
	{
		m_jsutil.ReadValue(std::forward<T>(t), v[0]);
	}

	template<typename T>
	typename std::enable_if<is_normal_class<T>::value>::type ReadValue(T&& t, Value& val, std::size_t M)
	{
		Value& p = val[Int2String[M]];
		ReadObject(t, p);
	}

	template<typename T>
	typename std::enable_if<is_basic_type<T>::value>::type ReadValue(T&& t, Value& val, std::size_t M)
	{
		m_jsutil.ReadValue(std::forward<T>(t), val);
	}

	template<typename T, size_t N>
	void ReadValue(T(&t)[N], Value& val, std::size_t M)
	{
		Value& p = val[Int2String[M]];

		ReadArray<T>(t, p);
	}

	template<typename T, size_t N>
	void ReadValue(std::array<T,N>& t, Value& val, std::size_t M)
	{
		Value& p = val[Int2String[M]];

		ReadArray<T>(t, p);
	}

private:
	JsonUtil m_jsutil;
};

