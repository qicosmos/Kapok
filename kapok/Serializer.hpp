#pragma once
#include "traits.hpp"
#include "Common.hpp"
#include "JsonUtil.hpp"

//严格按照字段的顺序进行序列化和反序列化, 定长数组
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

	//序列化普通的结构体，没有嵌套，没有容器数组
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

	//序列化自定义类型
	template<typename T>
	typename std::enable_if<is_user_class<T>::value>::type WriteObject(T&& t)
	{
		WriteTuple(t.Meta());
	}

	//序列化tuple
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

	/*************序列化容器*************/

	//序列化非map容器
	template<typename T>
	typename std::enable_if<is_singlevalue_container<T>::value>::type WriteObject(T&& t)
	{
		WriteArray(t);
	}

	//序列化适配器
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

	//序列化适配器
	template<typename T>
	typename std::enable_if<is_queue<T>::value>::type WriteObject(T&& t)
	{
		WriteAdapter(t, [&]{return t.front(); });
	}

	//序列化map容器
	/************************************/
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

	/*************序列化数组*************/

	//序列化std::array
	template<typename T, size_t N>
	void WriteObject(std::array<T, N>& t)
	{
		WriteArray(t);
	}

	//序列化定长数组
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

	/************************************/

	//序列化基本类型
	template<typename T>
	typename std::enable_if<is_basic_type<T>::value>::type WriteObject(T&& t)
	{
		m_jsutil.WriteValue(std::forward<T>(t));
	}

	//tuple和自定义类型可以合并
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

	//定长数组
	template <unsigned N, typename T>
	void WriteValue(T(&p)[N], std::size_t M)
	{
		SerializeImpl(p, Int2String[M]);
	}

private:
	JsonUtil m_jsutil;
};

