#ifndef __QY_LIST_H__
#define __QY_LIST_H__

#include "qyutil/qydefine.h"

DEFINE_NAMESPACE(qy)

template<class T>
class QyNodeT : public t_qyPOSITION
{
public:
	QyNodeT():prev(NULL),next( NULL){}
private:
	QyNodeT* prev;   /**< prev QyNodeT containing element */
	QyNodeT* next;   /**< next QyNodeT containing element */
	T element;      /**< element in Current node */

	template<class T> friend class QyList;
};

#define Position_node_cast(pos)   ((QyNodeT<T>*)pos)
#define Node_position_cast(node)  ((POSITION_l)node)

typedef void (*PNodeFreeCallBack)(void* element);

typedef int  (*PNodeQueryCallBack)(POSITION_l& position , unsigned int item , void* element , void* data);

/** */
template<class T>
class QyList
{	
public:
	QyList();
	virtual ~QyList();
public:
	/**
	* Free a list of element.
	* Each element will be free with the method given as the second parameter.
	* @param li The element to work on.
	* @param free_func The method that is able to release one element of the list.
	*/
	void free(PNodeFreeCallBack freeCB);

	/**
	* Get the size of a list of element.
	* @param li The element to work on.
	*/
	unsigned int size () const;
	/**
	 * @brief add an element in a list
	*/
	unsigned int push_back(T element) { return add(element); }
	/**
	* Add an element in a list.
	* @param li The element to work on.
	* @param element The pointer on the element to add.
	* @param inspos ,if inspos !=NULL , insert element to inspos before
	*/
	unsigned int add (T element , POSITION_l inspos = NULL);
	POSITION_l add_position(T element , POSITION_l inspos = NULL);

	/**
	* @param inspos ,if inspos !=NULL , insert element to inspos back
	*/
	POSITION_l add_back_position(T element , POSITION_l inspos = NULL);
	/**
	* Remove an element from a list.
	* @param li The element to work on.
	* @param pos the index of the element to remove.
	* @return QyNodeT<T>->element.
	*/
	T remove (unsigned int pos);
	T remove (POSITION_l node);

	/** move the node to dest node front */
	int move_front(POSITION_l node , POSITION_l destnode);

	/** move the node to dest node back */
	int move_back(POSITION_l node , POSITION_l destnode);

	/**
	 * query a list of element
	*/
	POSITION_l query( PNodeQueryCallBack queryCB , void* object, unsigned int* index = 0 , POSITION_l start = 0);
	POSITION_l prevQuery( PNodeQueryCallBack queryCB , void* object, unsigned int* index = 0 , POSITION_l start = 0);
	/**
	 * @brief 获取点
	*/
	POSITION_l at(unsigned int pos);
	/**
	* Get an element from a list.
	* @param pos the index of the element to get.
	* @return QyNodeT<T>->element.
	*/
	T  get (unsigned int pos);
	T  get (POSITION_l node);

	T* getptr (unsigned int pos);
	T* getptr (POSITION_l node);
	/**
	 * swap two element from this list
	 * @param srcNode source element
	 * @param destNode dest element
	*/
	int   swap(POSITION_l srcNode,POSITION_l destNode);
	/**
	 * @brief 开始一个结点
	 *
	 * @param li list of VLLIST
	 * @return 结点指针
	 */
	POSITION_l first();
	/**
	 * @brief 最后一个结点
	 *
	 * @param li list of VLLIST
	 * @return 结点指针
	 */
	POSITION_l last();
	/**
	 * @brief 下一个结点
	 *
	 * @param node node of QyNodeT<T>
	 * @return 结点指针
	 */
	POSITION_l next(const POSITION_l  node);
	/**
	 * @brief 前一个结点
	 *
	 * @param node node of QyNodeT<T>
	*/
	POSITION_l prev(const POSITION_l node);
private:
	void sub(void)
	{
		counts_--;
		if ((first_ == tail_) && (first_ == NULL))
		{
			counts_ = 0;
		}
	}
    POSITION_l add_init(T el , POSITION_l position);
protected:
	unsigned int counts_;         /**< Number of element in the list */
	QyNodeT<T>* first_;   /**< Next node containing element  */
	QyNodeT<T>* tail_;    /**< Next node containing element  */
};

template<class T>
QyList<T>::QyList()
{
	counts_ = 0;
	tail_ = first_ = 0;
}

template<class T>
QyList<T>::~QyList()
{
}

template<class T>
void QyList<T>::free( PNodeFreeCallBack freeCB)
{
	QyNodeT<T> *next = 0,*node = 0;
	next = Position_node_cast( first());	
	while( next)
	{
		if (freeCB)
		{
			freeCB((void*)next->element);
		}
		node = next ;
		next = next->next;
		node->prev = 0;
		node->next = 0;
		qyDeleteM(node);
	}
	counts_ = 0;
	tail_ = first_   = 0;
}

template<class T>
POSITION_l QyList<T>::prevQuery(PNodeQueryCallBack queryCB, void* object, unsigned int* index = 0, POSITION_l start = 0)
{
	uint count = 0;
	POSITION_l pos = 0;
	if (!start)
	{
		pos = last();
		count = size();
	}
	else
	{
		pos = start;
	}
	while( pos)
	{
		int retVal = queryCB( pos , count , get(pos) , object);
		if (0 == retVal)
		{
			if (index) 
			{
				*index = count;
			}
			return pos;
		}
		else if (-2 == retVal)
		{
			break;
		}
		pos = prev(pos);
		count--;
	}	
	if (index)
	{
		*index = -1;
	}
	return NULL;
}

template<class T>
POSITION_l QyList<T>::query(PNodeQueryCallBack queryCB, void* object, unsigned int* index = 0, POSITION_l start = 0)
{
	uint count = 0;
	POSITION_l pos = 0;
	if (!start)
	{
		pos = first();	
	}
	else
	{
		pos = start;
	}
	while (pos)
	{
		int retVal = queryCB( pos , count , get(pos) , object);
		if (0 == retVal)
		{
			if (index)
			{
				*index = count;
			}
			return pos;
		}
		else if (-2 == retVal)
		{
			break;
		}
		pos = next(pos);
		count++;
	}	
	if (index)
	{
		*index = -1;
	}
	return NULL;
}

template<class T>
unsigned int QyList<T>::size() const
{
	return counts_;
}

template<class T>
unsigned int QyList<T>::add (T el , POSITION_l position)
{
	unsigned int i = counts_;
	if (!add_position(el , position))
	{
		return -1;
	}
	return i;
}

template<class T>
POSITION_l QyList<T>::add_init(T el , POSITION_l position)
{
	QyNodeT<T> *newnode = 0;
	if (counts_ == 0)
	{
		newnode = new QyNodeT<T>();		
		if (newnode == NULL)
		{
			return NULL;
		}
		newnode->element = el;
		newnode->next = 0;
		newnode->prev = 0;
		tail_ = first_ = newnode;
	}
	else
	{
		QyNodeT<T> *insert = Position_node_cast(position);
		if (insert)
		{
			return NULL;
		}
		newnode = new QyNodeT<T>();
		if (!newnode)
		{
			return NULL;
		}
		QyNodeT<T> *node = Position_node_cast( last());
		newnode->element = el;
		newnode->next = NULL;
		newnode->prev = node;
		node->next = newnode;			
		tail_ = newnode;
	}
	counts_++;
	return Node_position_cast(newnode);
}

template<class T>
POSITION_l QyList<T>::add_position(T el , POSITION_l position)
{
	POSITION_l newpos = add_init(el , position);
	if (newpos)
	{
		return newpos;
	}
	QyNodeT<T> *insert = Position_node_cast(position);
	QyNodeT<T> *prev = 0;
	QyNodeT<T> *newnode = new QyNodeT<T>();
	if (!newnode) 
	{
		return NULL;
	}
	newnode->element = el;
	newnode->next    = 0;
	newnode->prev    = 0;
	//插入insert前
	prev  = insert->prev ;
	newnode->next = insert;
	newnode->prev = prev;
	if (prev)
	{
		prev->next= newnode;
	}
	insert->prev = newnode ;
	if (insert == first_)
	{
		first_ = newnode;
	}
	counts_++;
	return Node_position_cast(newnode);
}

template<class T>
POSITION_l QyList<T>::add_back_position(T el , POSITION_l inspos)
{
	POSITION_l newpos = add_init(el , inspos);
	if (newpos)
	{
		return newpos;
	}
	QyNodeT<T> *insert = Position_node_cast(inspos);
	QyNodeT<T> *next = 0;
	QyNodeT<T> *newnode = new QyNodeT<T>();
	if (!newnode) 
	{
		return NULL;
	}
	newnode->element = el;
	newnode->next    = 0;
	newnode->prev    = 0;
	
	next  = insert->next ;
	newnode->next = next;
	newnode->prev = insert;
	if (next)
	{
		next->prev = newnode;
	}
	insert->next = newnode ;
	if (insert == tail_)
	{
		tail_ = newnode;
	}
	counts_++;
	return Node_position_cast(newnode);
}

template<class T>
POSITION_l QyList<T>::at(unsigned int pos)
{
	unsigned int i = 0;
	QyNodeT<T> *node = 0;	
	if (pos < 0 || pos >= counts_)
	{
		return NULL;
	}
	node = first_;
	while (pos > i)
	{
		i++;
		node = (QyNodeT<T> *) node->next;
	}
	return node;
}

template<class T>
T QyList<T>::get(unsigned int pos)
{
	return get( at(pos));
}

template<class T>
T QyList<T>::get (POSITION_l position)
{
	return  (!position) ? NULL:Position_node_cast(position)->element;
}

template<class T>
T* QyList<T>::getptr (unsigned int pos)
{
	return  (!position) ? NULL : &Position_node_cast(position)->element;
}

template<class T>
T* QyList<T>::getptr (POSITION_l node)
{
	return  (!position) ? NULL : &Position_node_cast(position)->element;
}

template<class T>
int QyList<T>::swap(POSITION_l srcPos,POSITION_l destPos)
{
	T tmp_element;
	QyNodeT<T> *src_node=0,*dest_node=0;
	if (!srcPos || !destPos || srcPos == destPos)
	{
		return -1;	
	}
	src_node = Position_node_cast(srcPos);
	dest_node= Position_node_cast(destPos);	
	tmp_element       = src_node->element;
	src_node->element = dest_node->element;
	dest_node->element= tmp_element;
	return 0;
}

template<class T>
T QyList<T>::remove (unsigned int pos)
{
	QyNodeT<T> *node=0,*prev=0,*next=0;T ele;
	unsigned int i = 0;
	if (pos < 0 || pos >= counts_)
	{
		return NULL;
	}
	node = first_;
	while (i < pos && node)
	{
		i++;
		node = (QyNodeT<T> *) node->next;
	}
	prev = node->prev;
	next = node->next;
	ele  = node->element;
	if (prev)
	{
		prev->next = next;
	}
	if (next)
	{
		next->prev = prev;
	}
	if (node == first_)
	{
		first_ = next;
	}
	if (node == tail_)
	{
		tail_ = prev;
	}
	DeleteM_qy(node);

	sub();

	return (T)ele;
}

template<class T>
T QyList<T>::remove (POSITION_l position)
{
	QyNodeT<T> *prev=0,*next=0; T ele;
	QyNodeT<T> *node = Position_node_cast(position);
	if (node == NULL)
	{
		return NULL;
	}
	prev = node->prev;
	next = node->next;
	ele  = node->element;
	if (prev)
	{
		prev->next = next;
	}
	if (next)
	{
		next->prev = prev;
	}
	if (node == first_)
	{
		first_ = next;
	}
	if (node == tail_)
	{
		tail_ = prev;
	}
	qyDeleteM(node);
	sub();
	return ele;
}

template<class T>
int QyList<T>::move_front(POSITION_l position , POSITION_l destposition)
{
	QyNodeT<T> *node = Position_node_cast(position);
	QyNodeT<T> *destnode = Position_node_cast(destposition);
	if (NULL == node)
	{
		return -1;
	}
	if (position == destposition)
	{
		return 0;
	}
	if (NULL == destnode)
	{
		destnode = Position_node_cast( first());
		if (NULL == destnode)
		{
			return -1;
		}
	}
	// disconnect node
	QyNodeT<T> *prev = node->prev;
	QyNodeT<T> *next = node->next;
	node->prev = node->next = NULL;
	if (prev)
	{
		prev->next = next;
	}
	if (next)
	{
		next->prev = prev;
	}
	if(node == first_)
	{
		first_ = next;
	}
	if(node == tail_)
	{
		tail_ = prev;
	}
	prev = destnode->prev;

	if (prev)
	{
		prev->next = node;
		node->prev = prev;
	}

	node->next = destnode;
	destnode->prev = node;

	if (destnode == first_)
	{
		first_ = node;
	}
	return 0;
}

template<class T>
int QyList<T>::move_back(POSITION_l node , POSITION_l destnode)
{
	QyNodeT<T> *node = Position_node_cast(position);
	QyNodeT<T> *destnode = Position_node_cast(destposition);
	if (NULL == node)
	{
		return -1;
	}
	if (position == destposition)
	{
		return 0;
	}
	if (NULL == destnode)
	{
		destnode = Position_node_cast( last());
		if (NULL == destnode)
		{
			return -1;
		}
	}
	// disconnect node
	QyNodeT<T> *prev = node->prev;
	QyNodeT<T> *next = node->next;
	node->prev = node->next = NULL;
	if (prev)
	{
		prev->next = next;
	}
	if (next)
	{
		next->prev = prev;
	}
	if(node == first_)
	{
		first_ = next;
	}
	if(node == tail_)
	{
		tail_ = prev;
	}
	next = destnode->next;
	if (next)
	{
		next->prev = node;
		node->next = next;
	}

	node->prev = destnode;
	destnode->next = node;

	if (destnode == tail_)
	{
		tail_ = node;
	}
	return 0;
}

template<class T>
POSITION_l QyList<T>::first()
{
	return(counts_ <= 0 ) ? NULL : Node_position_cast(first_);
}

template<class T>
POSITION_l QyList<T>::last()
{
	return(counts_ <= 0 )  ? NULL : Node_position_cast(tail_);
}

template<class T>
POSITION_l QyList<T>::next(const POSITION_l node)
{
	return node?Node_position_cast( (Position_node_cast(node)->next)):NULL;
}

template<class T>
POSITION_l QyList<T>::prev(const POSITION_l node)
{
	return node?Node_position_cast( (Position_node_cast(node)->prev)):NULL;	
}

//--------------------------------------------------------------------------------//
template<typename T>
class QyLinkNode
{
public:
	QyLinkNode()
	{
		next = NULL;
	}
	T data;
	QyLinkNode<T>* next;
};

typedef int  (*funLinkNodeCallback)(void* data , void* userdata);

typedef void (*funLinkNodeFree)(void* data);

template<typename T>
void qyutil_destroy_linknodeT(QyLinkNode<T>* plink , funLinkNodeFree pfree)
{
	if (!plink)
		return ;

	QyLinkNode<T>* pnext = plink->next;
	while( pnext)
	{
		QyLinkNode<T>* ptemp = pnext;
		pnext = pnext->next;
		if (ptemp)
		{
			if (pfree)
			{
				pfree( (void*)(&ptemp->data));
			}
			delete ptemp; ptemp = NULL;
		}
	}
	plink->next = NULL;
}

/** 删除一个结点  */
template<typename T>
int qyutil_remove_linknodeT( QyLinkNode<T>* &plink , void* data , funLinkNodeCallback pcall , funLinkNodeFree pfree)
{
	if (!plink || !pcall)
		return -1;

	QyLinkNode<T>* pnext = plink;
	QyLinkNode<T>* pprev = plink;
	QyLinkNode<T>* pcurrent = NULL;

	while( pnext)
	{
		pcurrent = pnext;
		pnext    = pcurrent->next;

		if (0 == pcall( (void*)(&pcurrent->data) , data))
		{
			pprev->next = pnext;
			if (plink == pcurrent)
			{
				plink = pnext;
			}
			if (pfree)
			{
				pfree( (void)(&pcurrent->data));
			}
			delete pcurrent;
			return 0;
		}
		else
		{
			pprev = pcurrent;
		}
	}
	return 0;
}


template<typename T>
class QyLink
{
public:
	QyLink()
	{
		counts_ = 0;
		link_ = last_ = 0;
	}

	~QyLink()
	{
		destroy();
	}

	int counts(void) const
	{
		return counts_;
	}

	void destroy(funLinkNodeFree pfree = 0)
	{		
		if (!link_)
			return ;

		qyutil_destroy_linknodeT<T>( link_ , pfree);

		delete link_; link_ = 0;
		last_ = 0;
		counts_ = 0;
	}

	QyLinkNode<T>* link_;

	QyLinkNode<T>* last_;

	int counts_;
};


END_NAMESPACE(qy)

#endif /* __QY_LIST_H__ */