/*
	class P
	{
		INIT_PROPERTY

		PROPERTY(int, a)
		PROPERTY(int, b)
		PROPERTY(float, c)
	};

	void func()
	{
		set<P::b>(23);
		get<P::c>();
	}
*/
/*
#ifndef PROPERTY_H
#define PROPERTY_H

#define INIT_PROPERTY								\
	public:						    					\
	template<typename D>							\
	void set(typename D::property_type value)		\
	{}												\
	template<typename D>							\
	typename D::property_type get()					\
	{}												\

#define PROPERTY(type, name)						\
	private:										\
	public:											\
		type	_##name;							\
	public:											\
		struct name{								\
			typedef type property_type;				\
		};											\

#define PROPERTY_END(cl, name)						\
		template<typename D>									\
		void cl::set<typename name::property_type>(cl::name::property_type value){  \
			_##name = value;						\
		}											\
		template<typename D>									\
		cl::name::property_type cl::get<typename cl::name::property_type>(){			\
			return _##name;						\
		}\

#endif
*/
