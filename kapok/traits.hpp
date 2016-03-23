#pragma once
#include <memory>
#include <type_traits>
#include <vector>
#include <array>
#include <queue>
#include <stack>
#include <set>
using namespace std;

namespace detail
{
	template<typename T>
	using decay_t = typename std::decay<T>::type;

	//template<typename T> std::false_type static check_tuple_size(...);
	//template<typename T> std::true_type static  check_tuple_size(decltype(std::tuple_size<T>::value)*);
	template < template <typename...> class U, typename T >
	struct is_instantiation_of : std::false_type {};

	template < template <typename...> class U, typename... Args >
	struct is_instantiation_of< U, U<Args...> > : std::true_type {};
	
	template<typename T>
	struct is_tuple : is_instantiation_of<std::tuple, T>
	{
	};

	//has_begin_end
	template<typename T>
	struct has_begin_end
	{
	private:
		template<typename U> static auto Check(int) -> decltype(std::declval<U>().begin(), std::declval<U>().end(), std::true_type());
		template<typename U> static std::false_type Check(...);

	public:
		enum
		{
			value = std::is_same<decltype(Check<T>(0)), std::true_type>::value
		};
	};

	template <typename T>
	struct has_const_iterator
	{
	private:
		template<typename C> static std::true_type Check(typename C::const_iterator*);
		template<typename C> static std::false_type  Check(...); 
	public:
		enum
		{
			value = std::is_same<decltype(Check<T>(0)), std::true_type>::value
		}; 
	}; 

	template <typename T>
	struct has_mapped_type
	{
	private:
		template<typename C> static std::true_type Check(typename C::mapped_type*);
		template<typename C> static std::false_type  Check(...);
	public:
		enum
		{
			value = std::is_same<decltype(Check<T>(0)), std::true_type>::value
		};
	};

//#define HAS_XXX_TYPE(token)
//	template <typename T>struct has_##token{
//	private:
//	template<typename C> static std::true_type Check(typename C::token##*); 
//		template<typename C> static std::false_type  Check(...);
//	public:
//		enum
//		{
//			value = std::is_same<decltype(Check<T>(0)), std::true_type>::value
//		};
//	};
//
//	HAS_XXX_TYPE(const_iterator)
//	HAS_XXX_TYPE(mapped_type)

	template<typename T> struct is_poiner_extent : std::false_type{};
	template<typename T> struct is_poiner_extent<std::shared_ptr<T>> : std::true_type{};
	template<typename T> struct is_poiner_extent<std::unique_ptr<T>> : std::true_type{};
	template<typename T> struct is_poiner_extent<std::weak_ptr<T>> : std::true_type{};

//#define IS_SMART_POINTER(token)
//	template<typename T> struct is_poiner_extent<std::token##_ptr<T>> : std::true_type{}; 
//
//	IS_SMART_POINTER(shared)
//	IS_SMART_POINTER(unique)
//	IS_SMART_POINTER(weak)
}

template<typename T>
struct is_string : std::integral_constant<bool, std::is_same<detail::decay_t<T>, std::string>::value>{};

template <typename T>
struct is_container : public std::integral_constant<bool, detail::has_const_iterator<detail::decay_t<T>>::value&&detail::has_begin_end<detail::decay_t<T>>::value&&!is_string<T>::value>{};

template <typename T>
struct is_singlevalue_container : public std::integral_constant<bool, !std::is_array<detail::decay_t<T>>::value&&!detail::is_tuple<detail::decay_t<T>>::value && is_container<detail::decay_t<T>>::value&&!detail::has_mapped_type<detail::decay_t<T>>::value>{};

template <typename T>
struct is_map_container : public std::integral_constant<bool, is_container<detail::decay_t<T>>::value&&detail::has_mapped_type<detail::decay_t<T>>::value>{};

template<typename T>
struct is_normal_class : std::integral_constant<bool, std::is_class<detail::decay_t<T>>::value&&!is_string<T>::value>
{};

template<typename T>
struct is_basic_type : std::integral_constant<bool, std::is_arithmetic<detail::decay_t<T>>::value || is_string<T>::value>
{};

template<typename T>
struct is_smart_pointer : detail::is_poiner_extent<detail::decay_t<T>>{};

template<typename T>
struct is_pointer_ext : std::integral_constant<bool, std::is_pointer<detail::decay_t<T>>::value || is_smart_pointer<detail::decay_t<T>>::value>{};

template <typename T, template <typename...> class Template>
struct is_specialization_of : std::false_type {};

template <template <typename...> class Template, typename... Args>
struct is_specialization_of<Template<Args...>, Template> : std::true_type{};

template<typename T> struct is_tuple : is_specialization_of<detail::decay_t<T>, std::tuple>{};
template<typename T> struct is_queue : is_specialization_of<detail::decay_t<T>, std::queue>{};
template<typename T> struct is_stack : is_specialization_of<detail::decay_t<T>, std::stack>{};
template<typename T> struct is_set : is_specialization_of<detail::decay_t<T>, std::set> {};
template<typename T> struct is_multiset : is_specialization_of<detail::decay_t<T>, std::multiset> {};
template<typename T> struct is_unordered_set : is_specialization_of<detail::decay_t<T>, std::unordered_set> {};
template<typename T> struct is_priority_queue : is_specialization_of<detail::decay_t<T>, std::priority_queue>{};

//#define IS_TEMPLATE_CLASS(token)
//template<typename T> struct is_##token : is_specialization_of<detail::decay_t<T>, std::token##>{}; 
//
//IS_TEMPLATE_CLASS(tuple)
//IS_TEMPLATE_CLASS(queue)
//IS_TEMPLATE_CLASS(stack)
//IS_TEMPLATE_CLASS(priority_queue)

template<typename T>
struct is_container_adapter : std::integral_constant<bool, is_queue<T>::value || is_stack<T>::value || is_priority_queue<T>::value>
{
};

template<typename T>
struct is_user_class : std::integral_constant<bool, is_normal_class<T>::value&&!is_container_adapter<T>::value&&!is_container<T>::value&&!is_tuple<T>::value>
{};
