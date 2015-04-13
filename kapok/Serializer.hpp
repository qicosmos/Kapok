#pragma once
#include "traits.hpp"
#include "Common.hpp"
#include "JsonUtil.hpp"

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
	void Serialize(T&& t, const char* key)
	{	
		m_jsutil.Reset();
		SerializeImpl(std::forward<T>(t), key);
	}

private:
	template<typename T>
	void SerializeImpl(T&& t, const char* key)
	{
		m_jsutil.StartObject();
		m_jsutil.WriteValue(key);
		m_jsutil.StartArray();
		WriteObject(std::forward<T>(t));
		m_jsutil.EndArray();
		m_jsutil.EndObject();
	}

	template<typename T>
	typename std::enable_if<is_user_class<T>::value>::type WriteObject(T&& t)
	{
		WriteTuple(t.Meta());
	}

	template<typename T>
	typename std::enable_if<is_tuple<T>::value>::type WriteObject(T&& t)
	{
		WriteTuple(t);
	}

	template<std::size_t I = 0, typename Tuple>
	typename std::enable_if<I == std::tuple_size<Tuple>::value>::type WriteTuple(const Tuple& t)
	{
	}

	template<std::size_t I = 0, typename Tuple>
	typename std::enable_if<I < std::tuple_size<Tuple>::value>::type WriteTuple(const Tuple& t)
	{
		WriteValue(std::get<I>(t), I);
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
		int index = 0;
		for (size_t i = 0, size = v.size(); i < size; i++)
		{
			WriteValue(f(), index);
			v.pop();
			index++;
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
		for (auto& pair : t)
		{
			m_jsutil.StartObject();
			m_jsutil.WriteJson("0", pair.first); //key

			m_jsutil.WriteValue("1"); //value
			m_jsutil.StartArray();
			WriteObject(pair.second);
			m_jsutil.EndArray();
			m_jsutil.EndObject();
		}
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

	template<typename Array>
	inline void WriteArray(Array& v)
	{
		m_jsutil.StartArray();
		int index = 0;
		for (auto i : v)
		{
			WriteValue(i, index);
			index++;
		}
		m_jsutil.EndArray();
	}

	template<typename T>
	typename std::enable_if<is_basic_type<T>::value>::type WriteObject(T&& t)
	{
		m_jsutil.WriteValue(std::forward<T>(t));
	}

	template<typename T>
	typename std::enable_if<is_normal_class<T>::value>::type WriteValue(T&& t, std::size_t M)
	{
		SerializeImpl(std::forward<T>(t), Int2String[M]);
	}

	template<typename T>
	typename std::enable_if<is_basic_type<T>::value>::type WriteValue(T&& t, std::size_t M)
	{
		m_jsutil.WriteValue(std::forward<T>(t));
	}

	template <unsigned N, typename T>
	void WriteValue(T(&p)[N], std::size_t M)
	{
		SerializeImpl(p, Int2String[M]);
	}

private:
	JsonUtil m_jsutil;
};

