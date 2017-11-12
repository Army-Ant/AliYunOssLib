/*  
 * Copyright (c) 2015 ArmyAnt
 * 版权所有 (c) 2015 ArmyAnt
 *
 * Licensed under the BSD License, Version 2.0 (the License); 
 * 本软件使用BSD协议保护, 协议版本:2.0
 * you may not use this file except in compliance with the License. 
 * 使用本开源代码文件的内容, 视为同意协议
 * You can read the license content in the file "LICENSE" at the root of this project
 * 您可以在本项目的根目录找到名为"LICENSE"的文件, 来阅读协议内容
 * You may also obtain a copy of the License at 
 * 您也可以在此处获得协议的副本:
 * 
 *     http://opensource.org/licenses/BSD-3-Clause
 * 
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an AS IS BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * 除非法律要求或者版权所有者书面同意,本软件在本协议基础上的发布没有任何形式的条件和担保,无论明示的或默许的.
 * See the License for the specific language governing permissions and limitations under the License. 
 * 请在特定限制或语言管理权限下阅读协议
 */

#ifndef CLASS_PRIVATE_HANDLE_HPP_2015_11_13
#define CLASS_PRIVATE_HANDLE_HPP_2015_11_13

#include <map>
#include <exception>

namespace ArmyAnt {


/*	* @ author			: Jason
	* @ date			: 13/11/2015
	* @ last update	    : 23/05/2017
	* @ summary			: 此类用于保护性隐藏C++类中的私有成员
	* @ uncompleted		:
	* @ untested		:
    * @ issue           : Xcode 下AAJson类const异常对象，初始化时String类对象成员调用String的初始化时，在GetHandle的insert操作中崩溃，需要回归测试确认
	*/

//此类用于保护性隐藏C++类中的私有成员，将私有成员定义在cpp实现文件的另一个类中（public），而原类增加一个 const T_Handle handle 公有成员
//实现文件定义一个static本类实例，作为私有成员管理器，原类的函数体内通过handle和管理器来获取私有成员
//T_Out参数代表原类型，T_In参数代表包含私有成员的类型，T_Handle代表句柄的类型，可从任意整型中选择，默认为uint32
//使用时，不要把管理器实例或者T_In类的任何信息，暴露在cpp文件之外

template <class T_Out, class T_In>
class ClassPrivateHandleManager
{
public:
    ClassPrivateHandleManager():handleMap() {}
	~ClassPrivateHandleManager() {}

	//创建一个内部类实例，这通常是在建立外部实例时进行调用的
	void GetHandle(T_Out* src, T_In* newObject = new T_In());
	//销毁内部实例，这通常是与外部实例的析构一起进行的
	void ReleaseHandle(T_Out* src);
	void ReleaseHandleWithDelete(T_Out* src);

	//根据句柄获取外部实例，这通常是用于对C接口时的调用，C语言使用句柄
	T_Out* GetSourceByHandle(const T_In* in);
	//根据句柄获取内部实例，所有外部对象的公有函数，都需要调用此函数才能访问内部数据
	T_In* GetDataByHandle(const T_Out* out);

	//根据句柄获取内部实例，是GetDataByHandle的快捷调用法
	T_In* operator[](const T_Out* out);

	//内外实例以及句柄的表图
	std::map<T_Out*, T_In*> handleMap;


	AA_FORBID_COPY_CTOR(ClassPrivateHandleManager);
	AA_FORBID_ASSGN_OPR(ClassPrivateHandleManager);
};

/******************************** Source Code *********************************/

template <class T_Out, class T_In>
void ArmyAnt::ClassPrivateHandleManager<T_Out, T_In>::GetHandle(T_Out* src, T_In* newObject)
{
    //设置句柄，创建内部实例，并关联到外部实例
    if (handleMap.find(src) == handleMap.end())
    {
        handleMap.insert(std::make_pair(src, newObject));
    }
    else
        throw std::out_of_range("the handle has been existed");
}

template <class T_Out, class T_In>
void ArmyAnt::ClassPrivateHandleManager<T_Out, T_In>::ReleaseHandle(T_Out* src)
{
	auto ret = handleMap.find(src);
	//销毁内部实例，解除关联
	if (ret != handleMap.end())
	{
		handleMap.erase(src);
	}
}

template <class T_Out, class T_In>
void ArmyAnt::ClassPrivateHandleManager<T_Out, T_In>::ReleaseHandleWithDelete(T_Out* src)
{
	auto ret = handleMap.find(src);
	//销毁内部实例，解除关联
	if (ret != handleMap.end())
	{
		ArmyAnt::Fragment::AA_SAFE_DEL(ret->second);
		handleMap.erase(src);
	}
}

template <class T_Out, class T_In>
T_Out* ArmyAnt::ClassPrivateHandleManager<T_Out, T_In>::GetSourceByHandle(const T_In* in)
{
    for (auto i = handleMap.begin(); i != handleMap.end(); ++i)
    {
        if (i->second == in)
            return i->first;
    }
	return nullptr;
}

template <class T_Out, class T_In>
T_In* ArmyAnt::ClassPrivateHandleManager<T_Out, T_In>::GetDataByHandle(const T_Out* out)
{
	auto ret = handleMap.find(const_cast<T_Out*>(out));
	return ret == handleMap.end() ? nullptr : ret->second;
}

template <class T_Out, class T_In>
T_In* ArmyAnt::ClassPrivateHandleManager<T_Out, T_In>::operator[](const T_Out* out)
{
	return GetDataByHandle(out);
}

} // namespace ArmyAnt

#endif
