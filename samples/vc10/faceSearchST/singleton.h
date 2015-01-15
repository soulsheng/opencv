
#pragma once


#include <iostream>
using namespace std;
  


template < class T > 
class Singleton
{
protected:
	static T* ms_pSingleton;

public:
	Singleton( void )
	{
		assert( !ms_pSingleton );
		ms_pSingleton = static_cast< T* >( this );//static_cast 强制转化
	}

	~Singleton( void )
	{  
		assert( ms_pSingleton );  
		//ms_pSingleton = static_cast< T* >( this );
		ms_pSingleton = 0; 
	}

	static T& getSingleton( void )
	{	assert( ms_pSingleton );  return ( *ms_pSingleton ); }

	static T* getSingletonPtr( void )
	{ return ms_pSingleton; }
};


//////////////////Singleton定义
/**
* @class CSingleton
* @brief 实现类把该类作为友元, 并且把构造函数作为非公有
*         如:
*          Class C
*          {
*          friend class CSingleton<C>;
*          protected:
*              C(){};
*          }
*/
template<class T>
class CSingleton
{
public:
    static T* instance()
    {
        //double check. 锁前和锁后检测，保证效率和多线程正确性
        if (!m_pInstance)
        {
            //TODO: 加锁. CMutexGuard guard(m_lock);
  
            if (!m_pInstance)
            {
                //static T t;
                //m_pInstance = &t;
                m_pInstance = new T;
            }
        }
        return m_pInstance;
    };
  
protected:
    CSingleton(){}; //防止产生实例
    CSingleton(const CSingleton&){}; //防止拷贝构造另一个实例
    CSingleton &operator =(const CSingleton&){}; //防止赋值构造出另一个实例
    virtual ~CSingleton()
    {
        if (m_pInstance)
        {
			cout << "CSingleton call ~CSingleton() to release" << endl;
            delete m_pInstance;
            m_pInstance = NULL;
        }
    };
  
private:
    static T* m_pInstance; //类的唯一实例
    //TODO: 省略了互斥锁成员m_lock
};
