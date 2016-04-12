#include "stdafx.h"
#include <stdio.h>
#include "Filter.h"

Filter::Filter()
{
	bstart = true;
	isfirstTime = true;
	
}
Filter::~Filter()
{
}

Vector4 Filter::Exponential_sf( Vector4 *X, Vector4 *s_pre, float a )//��Ĭ�ϲ����ĺ��������в��ظ�ָ����Ĭ��ֵ��ֻ��������ָ��
{
	Vector4 s = 0.0;
	s.x = a*X->x + (1-a)*s_pre->x;
	s.y = a*X->y + (1-a)*s_pre->y;
	s.z = a*X->z + (1-a)*s_pre->z;
	
	s_pre->x = s.x;
	s_pre->y = s.y;
	s_pre->z = s.z;
	//s_pre = s;
	return s;
}

//�����Ʋ�����һ��ָ��ƽ���˲�
Vector4 Filter::Exponential_sf(  Vector4 *X,  Vector4 *s_pre, Vector4 *b1_pre,  float a, float v )//��Ĭ�ϲ����ĺ��������в��ظ�ָ����Ĭ��ֵ��ֻ��������ָ��
{
	Vector4 s = 0.0;
	Vector4 b1_cur = 0.0;
	//if(bstart)//��һ�ν���ʱ
	//{
	//	bstart = false;
	//	s_pre = X;

	//}

	s.x = a*X->x + ( 1-a )*(s_pre->x + b1_pre->x);
	s.y = a*X->y + ( 1-a )*(s_pre->y + b1_pre->y);
	s.z = a*X->z + ( 1-a )*(s_pre->z + b1_pre->z);

	b1_pre->x = v * (s.x - s_pre->x) + (1-v)*b1_pre->x;
	b1_pre->y = v * (s.y - s_pre->y) + (1-v)*b1_pre->y;
	b1_pre->z = v * (s.z - s_pre->z) + (1-v)*b1_pre->z;
	
	//b1_pre = b1_cur;
	s_pre->x = s.x;
	s_pre->y = s.y;
	s_pre->z = s.z;
	
	//s_pre = s;
	return s;
}
//s1 = Exponential_sf(X[i], s1_pre[i], b1_pre[i], a, v);	
//�����Ʋ�����һ��ָ��ƽ���˲�
Vector4 Filter::Exponential_sf(  Vector4 X,  Vector4 s_pre, Vector4 b1_pre,  float a, float v )//��Ĭ�ϲ����ĺ��������в��ظ�ָ����Ĭ��ֵ��ֻ��������ָ��
{
	Vector4 s = 0.0;
	Vector4 b1_cur = 0.0;
	//if(bstart)//��һ�ν���ʱ
	//{
	//	bstart = false;
	//	s_pre = X;

	//}

	s.x = a*X.x + ( 1-a )*(s_pre.x + b1_pre.x);
	s.y = a*X.y + ( 1-a )*(s_pre.y + b1_pre.y);
	s.z = a*X.z + ( 1-a )*(s_pre.z + b1_pre.z);

	b1_pre.x = v * (s.x - s_pre.x) + (1-v)*b1_pre.x;
	b1_pre.y = v * (s.y - s_pre.y) + (1-v)*b1_pre.y;
	b1_pre.z = v * (s.z - s_pre.z) + (1-v)*b1_pre.z;
	
	//b1_pre = b1_cur;
	s_pre.x = s.x;
	s_pre.y = s.y;
	s_pre.z = s.z;
	
	//s_pre = s;
	return s;
}



//�����Ʋ����Ķ���ָ��ƽ���˲�
//aԽ����ʱԽС����ǰ���ݵ�Ӱ��ԽС��vԽ����ʱԽС���Ա仯Խ����
void Filter::DExponential_sf( Vector4 *X,  Vector4 *s1_pre, Vector4 *s2_pre, Vector4 *b1_pre, Vector4 *b2_pre, float a, float v)//��Ĭ�ϲ����ĺ��������в��ظ�ָ����Ĭ��ֵ��ֻ��������ָ��
{
	
	Vector4 s1 = 0.0;

	s1 = Exponential_sf(X, s1_pre, b1_pre, a, v);		
	

	X->x = a*s1.x + (1-a)*(s2_pre->x + b2_pre->x);
	X->y = a*s1.y + (1-a)*(s2_pre->y + b2_pre->y);
	X->z = a*s1.z + (1-a)*(s2_pre->z + b2_pre->z);

	b2_pre->x = v * (X->x - s2_pre->x) + (1-v)*b2_pre->x;
	b2_pre->y = v * (X->y - s2_pre->y) + (1-v)*b2_pre->y;
	b2_pre->z = v * (X->z - s2_pre->z) + (1-v)*b2_pre->z;

	s2_pre->x = X->x;
	s2_pre->y = X->y;
	s2_pre->z = X->z;
	
}




void Filter::JitterRemoval_sf(Vector4 *X, Vector4 *X_pre, Vector4 *b1_pre, float a, float v )
{
	Vector4 s = 0.0;
	double distance = sqrt(pow((X->x - X_pre->x), 2) + pow((X->y - X_pre->y), 2) + pow((X->z - X_pre->z), 2));
	//if( distance > 0.02)
	{
//		s = Exponential_sf(X, X_pre, b1_pre, a, v);	//Ԥ���˲�
		s = Exponential_sf( X, b1_pre, a );			//��Ԥ���˲�
		X->x = s.x;
		X->y = s.y;
		X->z = s.z;
		
	}
				
	X_pre->x = X->x;
	X_pre->y = X->y;
	X_pre->z = X->z;	

}

