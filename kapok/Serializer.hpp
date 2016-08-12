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

	//template<typename T>
	//void Serialize(T const& t, const char* key = nullptr)
	//{	
	//	m_jsutil.Reset();
	//	if (key == nullptr)
	//	{
	//		WriteObject(t, std::true_type{});
	//	}
	//	else 
	//	{
	//		SerializeImpl(t, key);
	//	}
	//}

	template<typename T>
	void Serialize(const T& t, const char* key = nullptr)
	{
		m_jsutil.Reset();
		if (key == nullptr)
		{
			WriteObject(t, std::true_type{});
		}
		else
		{
			SerializeImpl(t, key);
		}
	}

private:
	template<typename T>
	void SerializeImpl(T const& t, const char* key)
	{
		m_jsutil.StartObject();
		m_jsutil.WriteValue(key);
		WriteObject(t, std::true_type{});
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

	template <typename T, typename BeginObjec>
	std::enable_if_t<is_optional<T>::value> WriteObject(T const& t, BeginObjec bj)
	{
		if (static_cast<bool>(t))
		{
			WriteObject(*t, bj);
		}
		else
		{
			WriteNull();
		}
	}

	//template<typename T>
	//typename std::enable_if<is_user_class<T>::value>::type WriteObject(T&& t)
	//{
	//	m_jsutil.StartObject();
	//	WriteTuple(t.Meta());
	//	m_jsutil.EndObject();
	//}

	// variant visitor
	template <typename BeginObj>
	struct serialize_visitor : boost::static_visitor<>
	{
		explicit serialize_visitor(Serializer& s, int which)
			: s_(s)
			, which_(which - 1)
		{

		}

		template <typename T>
		void operator() (T const& to_write) const
		{
			//s_.WriteObject(to_write, BeginObj{});
			
			s_.BeginWriteKV(std::to_string(which_).c_str(), to_write);
		}

		void operator() (boost::blank) const
		{
			throw;
		}

		Serializer& s_;
		int which_;
	};

	template <typename BeginObj, typename ... Args>
	void WriteObject(variant<Args...> const& v, BeginObj bj)
	{
		if (static_cast<bool>(v))
		{
			//WriteObject(*t, bj);
			boost::apply_visitor(serialize_visitor<BeginObj>{ *this, v.which() }, v);
		}
		else
		{
			WriteNull();
		}
	}

	template<typename T, typename BeginObjec>
	typename std::enable_if<is_user_class<T>::value>::type WriteObject(const T& t, BeginObjec)
	{
		m_jsutil.StartObject();
		WriteTuple(((T&)t).Meta());
		m_jsutil.EndObject();
	}

	template<typename T, typename BeginObjec>
	typename std::enable_if<is_tuple<T>::value>::type WriteObject(T const& t, BeginObjec)
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
		WriteObject(std::get<I>(t), std::false_type{});
		WriteTuple<I + 1>(t);
	}

	template<typename T, typename BeginObjec>
	typename std::enable_if<is_singlevalue_container<T>::value>::type WriteObject(T const& t, BeginObjec)
	{
		WriteArray(t);
	}

	template <typename Adaptor, typename F>
	auto WriteAdaptor(Adaptor&& adaptor, F get)
	{
		using adaptor_t = std::remove_cv_t<std::remove_reference_t<Adaptor>>;
		adaptor_t temp = std::forward<Adaptor>(adaptor);
		m_jsutil.StartArray();
		while (!temp.empty())
		{
			WriteObject(get(temp), std::false_type{});
			temp.pop();
		}
		m_jsutil.EndArray();
	}

	template <typename T, typename BeginObject>
	auto WriteObject(T&& t, BeginObject) -> std::enable_if_t<is_queue<T>::value>
	{
		WriteAdaptor(std::forward<T>(t), [](auto const& adaptor) { return adaptor.front(); });
	}

	template<typename T, typename BeginObject>
	auto WriteObject(T&& t, BeginObject) -> std::enable_if_t<is_stack<T>::value || is_priority_queue<T>::value>
	{
		WriteAdaptor(std::forward<T>(t), [](auto const& adaptor) { return adaptor.top(); });
	}

	template<typename T, typename BeginObject>
	typename std::enable_if<is_map_container<T>::value>::type WriteObject(T const& t, BeginObject)
	{
		m_jsutil.StartObject();
		for (auto const& pair : t)
		{
			WriteKV(boost::lexical_cast<std::string>(pair.first).c_str(), pair.second);
		}
		m_jsutil.EndObject();
	}

	template<typename T>
	typename std::enable_if<is_pair<T>::value>::type WriteObject(T const& t, std::true_type)
	{
		m_jsutil.StartObject();
		WriteKV(boost::lexical_cast<std::string>(t.first).c_str(), t.second);
		m_jsutil.EndObject();
	}

	template<typename T>
	typename std::enable_if<is_pair<T>::value>::type WriteObject(T const& t, std::false_type)
	{
		WriteKV(boost::lexical_cast<std::string>(t.first).c_str(), t.second);
	}

	template<typename T, size_t N, typename BeginObject>
	void WriteObject(std::array<T, N> const& t, BeginObject)
	{
		WriteArray(t);
	}

	template <typename T, size_t N, typename BeginObject>
	void WriteObject(T const(&p)[N], BeginObject)
	{
		WriteArray(p);
	}

	template <size_t N, typename BeginObject>
	void WriteObject(char const(&p)[N], BeginObject bj)
	{
		WriteObject((const char*)p, bj);
	}

	//template <size_t N>
	//void WriteObject(char const(&p)[N], std::false_type)
	//{
	//	WriteObject((const char*)p, bj);
	//}

	template<typename Array>
	inline void WriteArray(Array const& v)
	{
		m_jsutil.StartArray();
		for (auto const& i : v)
		{
			WriteObject(i, std::false_type{});
		}
		m_jsutil.EndArray();
	}

	template<typename T, typename BeginObject>
	typename std::enable_if<is_basic_type<T>::value>::type WriteObject(T const& t, BeginObject)
	{
		m_jsutil.WriteValue(t);
	}

	template <typename T, typename BeginObject>
	auto WriteObject(T const& val, BeginObject) ->std::enable_if_t<std::is_enum<
		std::remove_reference_t<std::remove_cv_t<T>>>::value>
	{
		using under_type = std::underlying_type_t<
			std::remove_reference_t<std::remove_cv_t<T>>>;
		m_jsutil.WriteValue(static_cast<under_type>(val));
	}

	void WriteObject(const char* t, std::true_type)
	{
		m_jsutil.WriteValue(t);
	}

	void WriteObject(const char* t, std::false_type)
	{
		m_jsutil.WriteValue(t);
	}

	//template<typename T>
	//typename std::enable_if<is_normal_class<T>::value>::type WriteValue(T&& t)
	//{
	//	WriteKV(boost::lexical_cast<std::string>(t.first).c_str(), t.second);
	//}

	template<typename V>
	void WriteKV(const char* k, V& v)
	{
		m_jsutil.WriteValue(k);
		WriteObject(v, std::true_type{});
	}

	template<typename V>
	void BeginWriteKV(const char* k, V& v)
	{
		m_jsutil.StartObject();
		WriteKV(k, v);
		m_jsutil.EndObject();
	}

	template<typename T, typename BeginObject>
	typename std::enable_if<is_basic_type<T>::value>::type WriteValue(T const& t, BeginObject)
	{
		m_jsutil.WriteValue(std::forward<T>(t));
	}

	void WriteNull()
	{
		m_jsutil.WriteNull();
	}

private:
	JsonUtil m_jsutil;
};

