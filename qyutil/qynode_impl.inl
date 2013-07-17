#include <assert.h>
#include <string>
#include <map>
//--------------------------------------------------------------------------------------------//
//
// namespace qyUtil
DEFINE_NAMESPACE(qy)

class QyTree;

/**
 * @brief 树的私有信息存取类
 * 
 * @author 2009-07-05
 *
*/
class QyTreePrivate
{
public:
	QyTreePrivate(){
	}
	~QyTreePrivate(){
		Clear();
	}
private:
	/** 添加一些私有信息[UI上的一些值]
	*  如一个界面，可以用同一个数据，但所需的表现的元素不一样，就可以通过个存起来
	*  需要时，通过接口取
	*
	* @param PNodeprivateCB 的回调用(主要是用来管理 pp值)
	*/
	int addPrivate(const char* key , QyTree::PrivateValue pp , PNodeprivateCB lpPrivateCB )
	{
		if( 0 == queryPrivate( key , NULL ) )
		{
			return -1;
		}
		TItem tm;
		tm.cb = lpPrivateCB;
		tm.p  = pp;
		items_[key] = tm ;

		return 0;
	}

	/** 删除这个私有信息
	*
	* @param pp if pp != NULL 可以删除是返回来,然后由外面对其pp进行处理
	*/
	int removePrivate(const char* key,QyTree::PrivateValue* pp)
	{
		itemMAP::iterator it = items_.begin();
		while( it != items_.end() )
		{
			if( (*it).first == key )
			{
				if( pp )
				{
					(*pp) = it->second.p;
				}
				items_.erase( it );
				return 0;
			}
			it++;
		}
		return -1;
	}
	/**
	* 查询相应的值.
	*/
	int queryPrivate(const char* key,QyTree::PrivateValue* pp)
	{
		itemMAP::iterator it = items_.begin();
		while( it != items_.end() )
		{
			if( (*it).first == key )
			{
				if(pp )
				{
					(*pp) = it->second.p;
				}
				return 0;
			}
			it++;
		}
		return -1;
	}

	void Clear()
	{
		itemMAP::iterator it = items_.begin();
		while( it != items_.end() )
		{
			TItem& tm = (*it).second;
			if( tm.cb )
				tm.cb( (it)->first.c_str() , tm.p );
			it++;
		}
		items_.clear();
	}
private:
	/// 一些私有属性的存取
	typedef struct tagItem
	{
		QyTree::PrivateValue p;
		PNodeprivateCB cb;
	}TItem;
	typedef std::map<std::string,TItem> itemMAP;
	itemMAP items_;

	/// id
	QyNodeID id_;

	friend class QyTree;
};

/////////////////////////////////////////////////////////////////////////
//
// static function
/** 操作函数 */
class QyTreeFunction
{
public:
	friend class QyTree;

	/** 根据条件查找 */
	static bool isfind(HTREENODE hFind,UINT nFlag,UINT nQFlag,PNodequeryCB pQueryCB,void* pQueryCondition){
		bool bIs = false;
		if( QyTree::TN_NULL == nFlag || hFind->getFlags(nFlag) )
		{		
			if( nQFlag == QyTree::QF_ROOT && hFind ->isRoot() )
				bIs = true;
			else if( nQFlag == QyTree::QF_NODE && !hFind ->isRoot() )
				bIs = true;
			else if( nQFlag == QyTree::QF_ALL )
				bIs = true;					

			if( bIs )
			{
				if( pQueryCB )
					return pQueryCB(hFind,pQueryCondition);			
			}
		}
		return bIs;
	}

	/** 查找子结点 */
	static HTREENODE getchildnode(HTREENODE hNode,UINT nFlag,UINT nQFlag,PNodequeryCB pQueryCB,void* pQueryCondition){
		//查找子项
		if( hNode && hNode->isHasChilds() )
		{	
			// 如果父结点,没有展开或者不可见,就不显示子项
			if( nFlag && 
				( nFlag & QyTree::TN_VISIBLE_BIT) && 
				( !hNode->getFlags(QyTree::TN_VISIBLE_BIT) || !hNode->getFlags(QyTree::TN_EXPAND_BIT) ) )
			{
				return NULL;
			}
			if( nQFlag == QyTree::QF_ROOT && hNode ->getRootCounts() <= 0 ) //只查询根
				return NULL;

			if( nQFlag == QyTree::QF_NODE && (hNode->getNodeCounts() - hNode->getRootCounts() )<= 0 ) //只查询结点
				return NULL;

			return hNode->getNextNode(NULL,nFlag,nQFlag,pQueryCB,pQueryCondition);
		}
		return NULL;
	}

	///
	/// 删除一项
	///
	static void DeleteLink(HTREENODE hNode){
		if ( !hNode )
		{
			return;
		}

		HTREENODE next=hNode->next_;
		HTREENODE prev=hNode->prev_;
		if( prev ) prev->next_ = next;
		if( next ) next->prev_ = prev;	
		if( hNode->parent_ && hNode == hNode->parent_->child_ )
		{
			hNode->prev_ = 0;
			if ( next )
			{
				next->prev_ = NULL;
			}
			hNode->parent_->child_ = hNode->next_;

		}
		hNode->parent_ = 0;
		hNode->prev_   = 0;
		hNode->next_   = 0;
		hNode->level_  = 0;
	}
	///
	/// 在hInsNode 项后插入一项hNode
	///
	static void InsertBack(HTREENODE hNode,HTREENODE hInsNode){
		HTREENODE hInNext = hInsNode->next_;

		HTREENODE hNext = hNode;
		while( (hNext->next_) ) 
			hNext = hNext->next_;

		hNode->level_   = hInsNode->level_;
		hInsNode->next_ = hNode;
		hNode->prev_    = hInsNode;
		hNext->next_    = hInNext;
		hNode->parent_  = hInsNode->parent_;
		if( hInNext ) hInNext->prev_ = hNode;
	}
	///
	/// 把hNode,放在hInsNode前面
	///
	static void InsertFront(HTREENODE hNode,HTREENODE hInsNode){
		HTREENODE hNext = hNode;
		while( (hNext->next_) ) 
			hNext = hNext->next_;

		HTREENODE hPrev   = hInsNode->prev_;
		hNode->prev_    = hPrev;
		hNext->next_    = hInsNode;
		hInsNode->prev_ = hNode;
		if( hPrev ) hPrev->next_ = hNode;
		if( hInsNode->parent_ && hInsNode == hInsNode->parent_->child_ ) {
			hInsNode->parent_->child_ = hNode;
		}
		hNode->parent_ = hInsNode->parent_;
		hNode->level_  = hInsNode->level_;
	}
	///
	/// 递归调用 hNode
	///
	static int RecurGetNode(HTREENODE hParent,HTREENODE hNode){
		if( !hParent || !hNode )
			return -1;

		HTREENODE hTmp = hParent->child_;
		if( !hTmp )
			return -1;   

		while( hTmp )
		{
			if( hTmp == hNode  )
				return 0;

			if( hTmp->isHasChilds() ) 
			{
				if( 0 == RecurGetNode(hTmp,hNode) )
					return 0;
			}
			hTmp = hTmp->next_;
		}
		return  -1;
	}
	/**
	* @brief 递归调用修改与这个结点有关联的结点
	*/
	static void modifyCounts(HTREENODE hNode,BOOL bAdd=TRUE){
		if( hNode && hNode->parent_ ) 
		{
			if ( hNode->isRoot() ) {
				if( !bAdd ) {
					hNode->parent_->modifyCounts( -(hNode->getNodeCounts()+1),-(hNode->getRootCounts()+1) );
				}else{
					hNode->parent_->modifyCounts(  (hNode->getNodeCounts()+1), (hNode->getRootCounts()+1) );
				}
			}else{
				if( !bAdd ) {
					hNode->parent_->modifyCounts(-1,0 );
				}else{
					hNode->parent_->modifyCounts( 1,0 );
				}
			}
		}
	}

	/**
	* @brief 修改自已与同包兄弟结点的父结点与级别信息 AND  统计计数
	*/
	static void ModifySiblingAndStatCount(HTREENODE hNode,
		const int nLevel, const HTREENODE hParent,int &nCounts,int &nRootCounts)
	{
		HTREENODE hTmp = hNode;
		while( hTmp )
		{
			hTmp->parent_ = hParent;
			hTmp->level_  = nLevel;

			if( hTmp->isHasChilds() )
			{
				nCounts += hTmp->getNodeCounts()+1;
				nRootCounts += hTmp->getRootCounts()+1;

				hTmp->modifyChildLevel();
			}
			else
				nCounts += 1;

			hTmp = hTmp->next_;
		}
	}
};

END_NAMESPACE(qy)
