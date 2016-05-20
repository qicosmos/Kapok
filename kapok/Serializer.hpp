#pragma once
#include <typeinfo>
#ifndef _MSC_VER
#include <cxxabi.h>
#endif
#include "traits.hpp"
#include "Common.hpp"
#include "JsonUtil.hpp"
#include <boost/lexical_cast.hpp>

class Serializer : NonCopyable
{
public:

	Serializer()
	{
	}

	~Serializer()
	{
	}

	const char* GetString()
	{
		return m_jsutil.GetJosnText();
	}

	template<typename T>
	void Serialize(T&& t, const char* key = nullptr)
	{	
		m_jsutil.Reset();
		if (key == nullptr)
		{
			WriteObject(t);
		}
		else 
		{
			SerializeImpl(t, key);
		}
	}

	template<typename T>
	void Serialize(const T& t, const char* key = nullptr)
	{
		m_jsutil.Reset();
		if (key == nullptr)
		{
			WriteObject(t);
		}
		else
		{
			SerializeImpl((T&)t, key);
		}
	}

private:
	template<typename T>
	void SerializeImpl(T&& t, const char* key)
	{
		m_jsutil.StartObject();
		m_jsutil.WriteValue(key);
		WriteObject(t);
		m_jsutil.EndObject();
	}

	template<typename T>
	const char* type_name()
	{
#ifndef _MSC_VER
		return abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
#else
		return typeid(T).name();
#endif
	}

	template<typename T>
	const char* get_type_name()
	{
		const char* name = type_name<T>();
		return has_space(name) ? "temp" : type_name<T>();
	}

	bool has_space(const char* str)
	{
		const size_t len = strlen(str);
		for (size_t i = 0; i < len; i++)
		{
			if (str[i] == ' ')
				return true;
		}

		return false;
	}

	template <typename T>
	std::enable_if_t<is_optional<T>::value> WriteObject(T const& t)
	{
		if (static_cast<bool>(t))
		{
			WriteObject(*t);
		}
	}

	//template<typename T>
	//typename std::enable_if<is_user_class<T>::value>::type WriteObject(T&& t)
	//{
	//	m_jsutil.StartObject();
	//	WriteTuple(t.Meta());
	//	m_jsutil.EndObject();
	//}

	template<typename T>
	typename std::enable_if<is_user_class<T>::value>::type WriteObject(const T& t)
	{
		m_jsutil.StartObject();
		WriteTuple(((T&)t).Meta());
		m_jsutil.EndObject();
	}

	template<typename T>
	typename std::enable_if<is_tuple<T>::value>::type WriteObject(T&& t)
	{
		m_jsutil.StartArray();
		WriteTuple(t);
		m_jsutil.EndArray();
	}

	template<std::size_t I = 0, typename Tuple>
	typename std::enable_if<I == std::tuple_size<Tuple>::value>::type WriteTuple(const Tuple& t)
	{
	}

	template<std::size_t I = 0, typename Tuple>
	typename std::enable_if<I < std::tuple_size<Tuple>::value>::type WriteTuple(const Tuple& t)
	{
		WriteObject(std::get<I>(t));
		WriteTuple<I + 1>(t);
	}

	template<typename T>
	typename std::enable_if<is_singlevalue_container<T>::value>::type WriteObject(T&& t)
	{
		WriteArray(t);
	}

	template<typename T>
	typename std::enable_if<is_stack<T>::value || is_priority_queue<T>::value>::type WriteObject(T&& t)
	{
		WriteAdapter(t, [&]{return t.top(); });
	}

	template<typename Adapter, typename F>
	inline void WriteAdapter(Adapter& v, F f)
	{
		m_jsutil.StartArray();
		for (size_t i = 0, size = v.size(); i < size; i++)
		{
			WriteValue(f());
			v.pop();
		}
		m_jsutil.EndArray();
	}

	template<typename T>
	typename std::enable_if<is_queue<T>::value>::type WriteObject(T&& t)
	{
		WriteAdapter(t, [&]{return t.front(); });
	}

	template<typename T>
	typename std::enable_if<is_map_container<T>::value>::type WriteObject(T&& t)
	{
		m_jsutil.StartObject();
		for (auto& pair : t)
		{
			WriteKV(boost::lexical_cast<std::string>(pair.first).c_str(), pair.second);
		}
		m_jsutil.EndObject();
	}

	template<typename T>
	typename std::enable_if<is_pair<T>::value>::type WriteObject(T&& t)
	{
		WriteKV(boost::lexical_cast<std::string>(t.first).c_str(), t.second);
	}

	template<typename T, size_t N>
	void WriteObject(std::array<T, N>& t)
	{
		WriteArray(t);
	}

	template <typename T, size_t N>
	void WriteObject(T(&p)[N])
	{
		WriteArray(p);
	}

	template <size_t N>
	void WriteObject(char(&p)[N])
	{
		WriteObject((const char*)p);
	}

	template<typename Array>
	inline void WriteArray(Array& v)
	{
		m_jsutil.StartArray();
		for (auto i : v)
		{
			WriteObject(i);
		}
		m_jsutil.EndArray();
	}

	template<typename T>
	typename std::enable_if<is_basic_type<T>::value>::type WriteObject(T&& t)
	{
		m_jsutil.WriteValue(std::forward<T>(t));
	}

	void WriteObject(const char* t)
	{
		m_jsutil.WriteValue(t);
	}

	template<typename T>
	typename std::enable_if<is_normal_class<T>::value>::type WriteValue(T&& t)
	{
		WriteKV(boost::lexical_cast<std::string>(t.first).c_str(), t.second);
	}

	template<typename V>
	void WriteKV(const char* k, V& v)
	{
		m_jsutil.WriteValue(k);
		WriteObject(v);
	}

	template<typename T>
	typename std::enable_if<is_basic_type<T>::value>::type WriteValue(T&& t)
	{
		m_jsutil.WriteValue(std::forward<T>(t));
	}

private:
	JsonUtil m_jsutil;
};

