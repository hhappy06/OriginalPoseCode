#ifndef _RANDOMFOREST_
#define _RANDOMFOREST_




// �������ڵ�
struct TreeNode			//14�ֽ�
{
    int  left;			//4�ֽ�
    int  right;			//4�ֽ�
    signed char ux, uy;
    signed char vx, vy;
    signed short c;
};


// Ҷ�ӽڵ�Value: ��λ���ĸ���
struct ClassDist
{
    unsigned char id;
    unsigned char cnt;
};


// Ҷ�ӽڵ�Value, 5���������������
struct NodeValue
{
    ClassDist v[5];
};


// ���ɭ��
class RandomForest
{
public:
    static const int TREE_COUNT = 3;						//@xu-li:3����
    static const int TREE_DEPTH = 18;						//@xu-li:18��?
    static const int NODE_COUNT = (1<<(TREE_DEPTH+1)) - 1;	//@xu-li:��TREE_DEPTH+1���Ƿ���Ҫ��1?
    static const int VALUE_COUNT = 835114;
    static const int FOREST_DATA_SIZE = 0x01f2af4a;

    RandomForest();
    ~RandomForest();
  //  BOOL BuildForest(const  char * pszData, const int size, cl_context & context, cl_command_queue &commandQueue);
	bool BuildForest(const char * pszData, const int size);
	const TreeNode  * Tree(int treeID) const;
    const TreeNode  * Node(int treeID, int nodeID) const;
    const NodeValue * Value(int valueID) const;
//private:
    TreeNode * m_ppTree[3]; //[TREE_COUNT][NODE_COUNT];
    NodeValue * m_pValue; //[VALUE_COUNT];
	//cl_mem m_cl_tree;
	//cl_mem m_cl_value;
	//cl_context  m_context;
	//cl_command_queue m_commandQueue;
};




#endif// _RANDOMFOREST_
