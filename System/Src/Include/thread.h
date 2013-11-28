#ifndef THREAD_H
#define THREAD_H

/* Пример использования:
class A : public Fr::Thread<A>{
public:
	A()
		:	Thread(&A::f1)
	{}

private:
	Fr::CPointer<A> f1(){
		return &A::f2;
	}

	Fr::CPointer<A> f2(){
			return &A::f1;
	}
};

int _tmain(int argc, _TCHAR* argv[]){
	A a;
	a.threadAction();
	return 0;
}
*/

namespace Fr{
	template<class T>
	struct CPointer;

	template<class T>
	struct TFPointer{
		typedef CPointer<T> (T::*FPointer)();
	};

	template <class T>
	struct CPointer{
		typedef typename TFPointer<T>::FPointer TFP;

		CPointer(TFP _p)
			:	p(_p)
		{}

		operator TFP(){
			return p;
		}

		TFP p;
	};

	class IThread{
		public:
			virtual void threadAction() = 0;
	};

	template<class T>
	class Thread : public IThread{
		CPointer<T> ptr;

	public:
	    Thread(typename TFPointer<T>::FPointer _ptr = nullptr)
			:	ptr(_ptr)
		{}

		virtual ~Thread(void){}

		virtual void threadAction(){
			if (ptr != nullptr)
				ptr = (reinterpret_cast<T*>(this)->*ptr)();
		}
	};
}
#endif
