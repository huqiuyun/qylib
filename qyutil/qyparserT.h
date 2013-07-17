///////////////////////////////////////////////////////
//	***             * * * *      ****        ****    //
//	***  	      *         *     **          **     //
//	***  	     *           *    **          **     //
//	***  	     *           *    **************     //
//	***	     **  *           *    **          **     //
//	***********   *         *     **          **     //
//	***********	    * * * *      ****        ****    // 
///////////////////////////////////////////////////////2009-08-20 @loach
/*
*
* 此库可以由读者用于学习之用,不能用于商业用途
* 如果用在商业用途,一块法律追究自己承担!
*
* 作者： loach.h ( 胡秋云)
*
* 时间： 2009-09-21
*/
DEFINE_NAMESPACE(qy)

class QyParserT;
class QyParserPrivateT;

/// 行分析回调函数
/// cspacing 分隔符号( , # ... )
typedef int (*QyPLineParseCallBackT)(QyParserT* lpCompiler , CharT cspacing);

/**
* @brief 字符行编译器接口类,方便快速的找到对应的值,牺牲内存,获取速度
* 根据你需要的接口进行编译
* 
* 使用时注意 : AddRef() 与 Release() 函数的调用
*/
class QYUTIL_API QyParserT
{
public:
	/// construcot
    QyParserT();
	/// destructor
    ~QyParserT();
public:
	/**
	 * @brief 处理数据
	*/
	int     Handle(const CharT* value,bool bDefCB = true );
	/**
	 * @brief 需要处理的数据
	*/
	int     Handle(const CharT* lpszTxt ,size_t len,bool bDefCB = true );
	/**
	 * @brief 处理
	 * @param pCB 回调函数
	 * @param cspacing 分隔符号
	*/
	int     HandleCB(QyPLineParseCallBackT pCB , CharT cspacing = L',' );

	/**
	 * @brief 有多少参数值 
	*/
	size_t  Counts(void) const;

	/**
	* @brief 获取字符类型值
	* @param nRow 索引值
	* @param lpszDefault 默认值
	*/
	const CharT*  String(size_t nRow , int& len,const CharT *lpszDefault = NULL);
	/**
	* @brief 获取整数类型值
	* @param nRow 索引值
	* @param nDefault 默认值
	*/
	int    Int(size_t nRow , int nDefault = 0);

	/**
	* @brief 获取双倍浮点数类型值
	* @param nRow 索引值
	* @param dDefault 默认值
	*/
	double Double(size_t nRow , double dDefault = 0.0);
	/**
	* @brief 值
	*/
    const CharT* c_str(void);
	/**
	 * @brief 长度
	*/
	size_t Length(void);
	/**
	 * @brief 清空
	*/
    void  Clear();

	/*
	 * @brief 位置点
	*/
	void push(short row);
private:
	friend class QyParserPrivateT;
	QyParserPrivateT* d_ptr_;
};

END_NAMESPACE(qy)
