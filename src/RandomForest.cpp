#include "stdafx.h"
//#include <CL/cl.h>
#include "RandomForest.h"
//#include "clUtil.h"

typedef struct  
{
	UINT ind;
	UCHAR v[5];
} tmpNodeValueStorage;


RandomForest::RandomForest()
	: m_pValue(NULL)
	//m_cl_tree(NULL),
	//m_context(NULL)
{
}


RandomForest::~RandomForest()
{
	//if (m_ppTree)
	//{
	//	for (int i = 0; i < TREE_COUNT; i ++)
	//	{
	//		if (m_ppTree[i])
	//		{
	//			delete [] m_ppTree[i];
	//			m_ppTree[i] = NULL;
	//		}
	//	}
	//	delete [] m_ppTree;
	//	// m_ppTree = NULL;
	//}

	delete [] m_ppTree[0];
	if (m_pValue)
	{
		delete [] m_pValue;
		m_pValue = NULL;
	}
}


bool RandomForest::BuildForest(const  char * pszData, const int size)
{
	// check input
	if (!pszData || FOREST_DATA_SIZE != size)
	{
		return FALSE;
	}
//	 initialize tree structures
	//if (m_ppTree == NULL)
	//{
	//    m_ppTree = new TreeNode *[TREE_COUNT];
	//    if (m_ppTree == NULL)
	//    {
	//        return FALSE;
	//    }


	memset(m_ppTree, 0, TREE_COUNT * sizeof(TreeNode *));//@xu-li: sizeof(TreeNode *)�ڵ��С������m_ppTree�Ǹ����飬��������ָ���������ĸ��ڵ��ָ�룬��ֻ�������ڵ�Ĵ�С

	 //for (int i = 0; i < TREE_COUNT; i ++)
	 //{
	TreeNode * t = new TreeNode[NODE_COUNT * 3];
	if (t == NULL)
	{
		return FALSE;
	}

	m_ppTree[0] = t;
	m_ppTree[1] = t + NODE_COUNT;
	m_ppTree[2] = t + NODE_COUNT * 2;

	//}
	//}

	if (m_pValue == NULL)
	{
		m_pValue = new NodeValue[VALUE_COUNT];
		if (m_pValue == NULL)
		{
			return FALSE;
		}
	}


	const char * pForestData = pszData;
	for (int i=0; i<TREE_COUNT; i++)
	{
		TreeNode * n = m_ppTree[i];
		int ind = -1;
		while (++ind < NODE_COUNT)
		{
			TreeNode * src = (TreeNode *)pForestData;//@xu-li:ǿ��ת�����ı�ָ�����ͣ�ֻ�Ը�ֵ������Ӱ��
			n[ind].left = src->left;
			n[ind].right = src->right;
			n[ind].ux = src->ux;
			n[ind].uy = src->uy;
			n[ind].vx = src->vx;
			n[ind].vy = src->vy;
			n[ind].c  = src->c;
			pForestData += 0x10;					//@xu-li:pForestData��const char����ָ�룬һ���ƶ�һ���ֽڣ������ǿ��ת�������ı�ָ������
		}											//@xu-li:�ڵ����ݽṹ������ʵ����ֻ��14���ֽڣ���pForestDataÿ�μ�16���ֽڣ�˵��ģ�����ݴ洢ʱ�п���
		ASSERT(ind == NODE_COUNT);
		if (ind != NODE_COUNT)
		{
			return FALSE;
		}
	}

	{
		NodeValue * v = m_pValue;
		int ind = -1;
		while (++ind < VALUE_COUNT)
		{
			tmpNodeValueStorage * src = (tmpNodeValueStorage *)pForestData;
			UINT value_indices = src->ind;
			v[ind].v[0].id  = (unsigned char)(value_indices & 0x0000001F);//@xu-li:5λ��id
			v[ind].v[0].cnt = src->v[0];
			value_indices >>= 5;
			v[ind].v[1].id  = (unsigned char)(value_indices & 0x0000001F);
			v[ind].v[1].cnt = src->v[1];
			value_indices >>= 5;
			v[ind].v[2].id  = (unsigned char)(value_indices & 0x0000001F);
			v[ind].v[2].cnt = src->v[2];
			value_indices >>= 5;
			v[ind].v[3].id  = (unsigned char)(value_indices & 0x0000001F);
			v[ind].v[3].cnt = src->v[3];
			value_indices >>= 5;
			v[ind].v[4].id  = (unsigned char)(value_indices & 0x0000001F);
			v[ind].v[4].cnt = src->v[4];
			pForestData += 9;
		}
		ASSERT(ind == VALUE_COUNT);
		if (ind != VALUE_COUNT)
		{
			return FALSE;
		}
	}
	if (pForestData != pszData + FOREST_DATA_SIZE)
	{
		return FALSE;
	}



	return TRUE;
}

//BOOL RandomForest::BuildForest(const  char * pszData, const int size, cl_context & context, cl_command_queue &commandQueue)
//{
//	// check input
//	if (!pszData || FOREST_DATA_SIZE != size)
//	{
//		return FALSE;
//	}
//	if(!context)
//	{
//		return FALSE;
//	}
//	cl_int  ret;
//	cl_int iErrNum;
//	m_context = context;
//	m_commandQueue = commandQueue;
//	m_cl_tree = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR , 
//		sizeof(TreeNode) * NODE_COUNT * 3,   NULL, &iErrNum);
//	clCHECKERROR(iErrNum, CL_SUCCESS);
//
//	m_cl_value = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR ,
//		sizeof(NodeValue) * VALUE_COUNT,    NULL, &iErrNum);
//	clCHECKERROR(iErrNum, CL_SUCCESS);
//
//	TreeNode *t = (TreeNode *)clEnqueueMapBuffer(commandQueue, m_cl_tree, CL_TRUE, CL_MAP_WRITE, 0, 
//		sizeof(TreeNode) * NODE_COUNT * 3, 0, NULL, NULL, &ret);
//	clCHECKERROR(ret, CL_SUCCESS);
//	
//	m_pValue = (NodeValue *)clEnqueueMapBuffer(commandQueue, m_cl_value, CL_TRUE, CL_MAP_WRITE, 0, 
//		sizeof(NodeValue) * VALUE_COUNT, 0, NULL, NULL, &ret);
//	clCHECKERROR(ret, CL_SUCCESS);
//	// initialize tree structures
//	//if (m_ppTree == NULL)
//	//{
//	//    m_ppTree = new TreeNode *[TREE_COUNT];
//	//    if (m_ppTree == NULL)
//	//    {
//	//        return FALSE;
//	//    }
//
//
//	//memset(m_ppTree, 0, TREE_COUNT * sizeof(TreeNode *));
//
//	// for (int i = 0; i < TREE_COUNT; i ++)
//	// {
//	//TreeNode * t = new TreeNode[NODE_COUNT * 3];
//	//if (t == NULL)
//	//{
//	//	return FALSE;
//	//}
//	m_ppTree[0] = t;
//	m_ppTree[1] = t + NODE_COUNT;
//	m_ppTree[2] = t + NODE_COUNT * 2;
//
//
//	//}
//	//}
//
//	//if (m_pValue == NULL)
//	//{
//	//	m_pValue = new NodeValue[VALUE_COUNT];
//	//	if (m_pValue == NULL)
//	//	{
//	//		return FALSE;
//	//	}
//	//}
//
//
//	const char * pForestData = pszData;
//	for (int i=0; i<TREE_COUNT; i++)
//	{
//		TreeNode * n = m_ppTree[i];
//		int ind = -1;
//		while (++ind < NODE_COUNT)
//		{
//			TreeNode * src = (TreeNode *)pForestData;
//			n[ind].left = src->left;
//			n[ind].right = src->right;
//			n[ind].ux = src->ux;
//			n[ind].uy = src->uy;
//			n[ind].vx = src->vx;
//			n[ind].vy = src->vy;
//			n[ind].c  = src->c;
//			pForestData += 0x10;
//		}
//		ASSERT(ind == NODE_COUNT);
//		if (ind != NODE_COUNT)
//		{
//			return FALSE;
//		}
//	}
//
//	{
//		NodeValue * v = m_pValue;
//		int ind = -1;
//		while (++ind < VALUE_COUNT)
//		{
//			tmpNodeValueStorage * src = (tmpNodeValueStorage *)pForestData;
//			UINT value_indices = src->ind;
//			v[ind].v[0].id  = (unsigned char)(value_indices & 0x0000001F);
//			v[ind].v[0].cnt = src->v[0];
//			value_indices >>= 5;
//			v[ind].v[1].id  = (unsigned char)(value_indices & 0x0000001F);
//			v[ind].v[1].cnt = src->v[1];
//			value_indices >>= 5;
//			v[ind].v[2].id  = (unsigned char)(value_indices & 0x0000001F);
//			v[ind].v[2].cnt = src->v[2];
//			value_indices >>= 5;
//			v[ind].v[3].id  = (unsigned char)(value_indices & 0x0000001F);
//			v[ind].v[3].cnt = src->v[3];
//			value_indices >>= 5;
//			v[ind].v[4].id  = (unsigned char)(value_indices & 0x0000001F);
//			v[ind].v[4].cnt = src->v[4];
//			pForestData += 9;
//		}
//		ASSERT(ind == VALUE_COUNT);
//		if (ind != VALUE_COUNT)
//		{
//			return FALSE;
//		}
//	}
//	if (pForestData != pszData + FOREST_DATA_SIZE)
//	{
//		return FALSE;
//	}
//
//	ret = clEnqueueUnmapMemObject(commandQueue, m_cl_tree, t, 0, NULL, NULL);
//	clCHECKERROR(ret, CL_SUCCESS);
//	ret = clEnqueueUnmapMemObject(commandQueue, m_cl_value, m_pValue, 0, NULL, NULL);
//	clCHECKERROR(ret, CL_SUCCESS);
//
//
//	t = (TreeNode *)clEnqueueMapBuffer(commandQueue, m_cl_tree, CL_TRUE, CL_MAP_READ, 0, 
//		sizeof(TreeNode) * NODE_COUNT * 3, 0, NULL, NULL, &ret);
//	clCHECKERROR(ret, CL_SUCCESS);
//	m_ppTree[0] = t;
//	m_ppTree[1] = t + NODE_COUNT;
//	m_ppTree[2] = t + NODE_COUNT * 2;
//
//	m_pValue = (NodeValue *)clEnqueueMapBuffer(commandQueue, m_cl_value, CL_TRUE, CL_MAP_READ, 0, 
//		sizeof(NodeValue) * VALUE_COUNT, 0, NULL, NULL, &ret);
//	clCHECKERROR(ret, CL_SUCCESS);
//
//	return TRUE;
//}


const TreeNode * RandomForest::Tree(int treeID) const
{
	if (treeID < 0 || treeID >= TREE_COUNT)
	{
		return NULL;
	}
	else
	{
		return m_ppTree[treeID];
	}
}


const TreeNode * RandomForest::Node(int treeID, int nodeID) const
{
	if (treeID < 0 || treeID >= TREE_COUNT ||
		nodeID < 0 || nodeID >= NODE_COUNT)
	{
		return NULL;
	}
	else
	{
		return &(m_ppTree[treeID][nodeID]);
	}
}


const NodeValue * RandomForest::Value(int valueID) const
{
	if (valueID < 0 || valueID >= VALUE_COUNT)
	{
		return NULL;
	}
	else
	{
		return &(m_pValue[valueID]);
	}
}




