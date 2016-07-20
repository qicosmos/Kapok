#pragma once
#include "JsonUtil.hpp"
#include "traits.hpp"
#include <boost/lexical_cast.hpp>

class DeSerializer : NonCopyable
{
public:
	DeSerializer() = default;
	DeSerializer(const char* jsonText, std::size_t length)
	{
		Parse(jsonText, length);
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
		Parse(jsonText.c_str(), jsonText.length());
	}

	void Parse(const char* jsonText, std::size_t length)
	{
		m_jsutil.Parse(jsonText, length);
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

		ReadObject(t, jsonval, std::true_type{});
	}

	template<typename T>
	void Deserialize(T& t, bool has_root = false)
	{
		Value& jsonval = GetRootValue(nullptr, has_root);

		ReadObject(t, jsonval, std::true_type{});
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
    Value& GetRootValue(const char* key, bool has_root = true)
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
			ReadValue(value, jsonval[i], std::true_type{});
			t[i] = value;
		}
	}

	template<typename T, typename BeginObject>
	typename std::enable_if<is_tuple<T>::value>::type ReadObject(T& t, Value& val, BeginObject bo)
	{
		ReadTuple(std::forward<T>(t), val, bo);
	}

	template<typename T, typename BeginObject>
	typename std::enable_if<is_user_class<T>::value>::type ReadObject(T& t, Value& val, BeginObject)
	{
		ReadTuple(t.Meta(), val, std::false_type{});
	}

	template <typename T, typename BeginObject>
	auto ReadObject(T& t, Value& val, BeginObject) -> std::enable_if_t<is_enum<
		std::remove_cv_t<std::remove_reference_t<T>>>::value>
	{
		using under_type = std::underlying_type_t<std::remove_cv_t<std::remove_reference_t<T>>>;
		ReadObject(reinterpret_cast<under_type&>(t), val, std::true_type{});
	}

	template <typename T, typename BeginObject>
	auto ReadObject(T& t, Value& val, BeginObject) -> std::enable_if_t<is_optional<T>::value>
	{
		if (!val.IsNull())
		{
			std::remove_reference_t<decltype(*t)> tmp;
			ReadObject(tmp, val, std::true_type{});
			t = tmp;
		}
	}

	template<typename Tuple, typename BeginObject>
	void ReadTuple(Tuple&& tp, Value& val, BeginObject bo)
	{
		const int size = std::tuple_size<Tuple>::value;
		
		for (SizeType j = 0; j < size; j++)
		{
			SetValueByIndex(j, tp, val, bo);
		}
	}

	template<size_t k=0, typename Tuple, typename BeginObject>
	inline auto SetValueByIndex(size_t, Tuple&, Value&, BeginObject) ->
		std::enable_if_t<(k == std::tuple_size<Tuple>::value)>
	{
		throw std::invalid_argument("arg index out of range");
	}

	template<size_t k = 0, typename Tuple, typename BeginObject>
	inline auto SetValueByIndex(size_t index, Tuple& tp, Value& val, BeginObject bo) ->
		std::enable_if_t<(k < std::tuple_size<Tuple>::value)>
	{
		if (k == index)
		{
			ReadValue<k>(tp, val, bo);
		}
		else
		{
			SetValueByIndex<k + 1>(index, tp, val, bo);
		}
	}

	template<typename T, typename BeginObject>
	auto ReadObject(T&& t, Value& v, BeginObject) ->
		std::enable_if_t<is_stack<T>::value>
	{
		using U = typename std::decay<T>::type;

		size_t sz = v.Size();
		for (SizeType i = sz; i > 0; i--)
		{
			typename U::value_type value;
			ReadObject(value, v[i - 1], std::true_type{});
			push(t, value, i-1);
		}
	}

	template<typename T, typename BeginObject>
	auto ReadObject(T&& t, Value& v, BeginObject) ->
		std::enable_if_t<is_singlevalue_container<T>::value || is_container_adapter<T>::value>
	{
		using U = typename std::decay<T>::type;

		size_t sz = v.Size();
		for (SizeType i = 0; i < sz; i++)
		{
			typename U::value_type value;
			ReadObject(value, v[i], std::true_type{});
			push(t, value, i);
		}
	}

	template<typename T, typename BeginObject>
	auto ReadObject(T&& t, Value& v, BeginObject) ->
		std::enable_if_t<std::is_array<T>::value || is_std_array<T>::value>
	{
		using U = typename std::decay<T>::type;

		size_t sz = v.Size();
		for (SizeType i = 0; i < sz; i++)
		{
			typename U::value_type value;
			ReadObject(value, v[i], std::true_type{});
			t[i] = value;
		}
	}

	template<typename T, typename value_type>
	auto push(T& t, value_type& v, std::size_t index) ->
		std::enable_if_t<is_singlevalue_container<T>::value&&!is_set<T>::value&&!is_multiset<T>::value&&!is_unordered_set<T>::value>
	{
		t.push_back(v);
	}

	template<typename T, typename value_type>
	auto push(T& t, value_type& v, std::size_t index) ->
		std::enable_if_t<is_set<T>::value || is_multiset<T>::value || is_unordered_set<T>::value>
	{
		t.insert(v);
	}

	template<typename T, typename value_type>
	auto push(T& t, value_type& v, std::size_t index) ->
		std::enable_if_t<is_std_array<T>::value || std::is_array<T>::value>
	{
		t[index] = v;
	}

	template<typename T, typename value_type>
	auto push(T& t, value_type& v, std::size_t index) ->
		std::enable_if_t<is_container_adapter<T>::value || is_stack<T>::value>
	{
		t.push(v);
	}

	template<typename T, typename BeginObject>
	auto ReadObject(T&& t, Value& v, BeginObject) -> std::enable_if_t<is_map_container<T>::value>
	{
		using U = typename std::decay<T>::type;

		for (rapidjson::Value::ConstMemberIterator it = v.MemberBegin(); it != v.MemberEnd(); it++)
		{
			using key_type = typename U::key_type;
			using val_type = typename U::value_type::second_type;
			key_type key;
			val_type value;

			key = boost::lexical_cast<key_type>(it->name.GetString());

			ReadObject(value, (Value&)it->value, std::true_type {});

			t.emplace(key, value);
		}
	}

	template<typename T, typename BeginObject>
	auto ReadObject(T&& t, Value& v, BeginObject) -> std::enable_if_t<is_basic_type<T>::value>
	{
		m_jsutil.ReadValue(std::forward<T>(t), v);
	}

	template<size_t N=0, typename T, typename BeginObject>
	auto ReadValue(T&& t, Value& val, BeginObject) -> std::enable_if_t<is_user_class<T>::value>
	{
		ReadObject(t, val, std::false_type{});
	}

	template<size_t N = 0, typename T, typename BeginObject>
	auto ReadValue(T&& t, Value& val, BeginObject) -> std::enable_if_t<is_optional<T>::value>
	{
		ReadObject(t, val, std::true_type{});
	}

	template <size_t N = 0, typename T, typename BeginObject>
	auto ReadValue(T&& t, Value& val, BeginObject) -> std::enable_if_t<std::is_enum<
		std::remove_cv_t<std::remove_reference_t<T>>>::value>
	{
		using under_type = std::underlying_type_t<std::remove_cv_t<std::remove_reference_t<T>>>;
		ReadObject(std::forward<under_type>(t), val, std::true_type{});
	}

	template<size_t N = 0, typename T>
	auto ReadObject(T&& t, Value& val, std::true_type bo) -> std::enable_if_t<is_pair<T>::value>
	{
		using pair_t = std::remove_cv_t<std::remove_reference_t<T>>;
		using first_type = typename pair_t::first_type;
		using second_type = typename pair_t::second_type;

		//assert(val.MemberCount() == 1);
		if(val.MemberCount()!=1)
			throw std::invalid_argument("member count error");

		auto itr = val.MemberBegin();
		t.first = boost::lexical_cast<first_type>(itr->name.GetString());
		ReadObject(t.second, itr->value, bo);
	}

	template<size_t N = 0, typename T>
	auto ReadObject(T&& t, Value& val, std::false_type bo) -> std::enable_if_t<is_pair<T>::value>
	{
		ReadObject(t.second, val, bo);
	}

	template<size_t N = 0, typename T, typename BeginObject>
	auto ReadValue(T&& t, Value& val, BeginObject) -> std::enable_if_t<is_basic_type<T>::value>
	{
		m_jsutil.ReadValue(std::forward<T>(t), val);
	}

	template<size_t N = 0, typename T>
	auto ReadValue(T&& t, Value& val, std::false_type bo) -> std::enable_if_t<is_tuple<T>::value>
	{
		//assert(!val.IsArray());
		if(val.IsArray())
			throw std::invalid_argument("should not be array");

		decltype(auto) tuple_elem = std::get<N>(t);
		auto itr = val.FindMember(tuple_elem.first);
		if(itr==val.MemberEnd())
			throw std::invalid_argument("the key is not exist");

		//assert(itr != val.MemberEnd());
		ReadObject(tuple_elem, (Value&)(itr->value), bo);
	}

	template<size_t N = 0, typename T>
	auto ReadValue(T&& t, Value& val, std::true_type bo) -> std::enable_if_t<is_tuple<T>::value>
	{
		//assert(val.IsArray());
		if (!val.IsArray())
			throw std::invalid_argument("should be array");
		ReadObject(std::get<N>(t), val[N], bo);
	}

private:
	JsonUtil m_jsutil;
};

