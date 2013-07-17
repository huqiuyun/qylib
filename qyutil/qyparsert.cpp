#include "qyosstring.h"

DEFINE_NAMESPACE(qy)

/// 分析的默认回调函数
int DefLineParseCallbackT(QyParserT* lpCompiler,CharT cspacing)
{
	short nRow   = 0;
	size_t   length = lpCompiler->Length();
	CharT *lpstr = const_cast<CharT*>(lpCompiler->c_str());
	if ( lpstr && length > 0 )
	{
		lpCompiler->push(nRow);
		while( 1 )
		{
			if ( nRow >= length )
			{
				lpCompiler->push(nRow);
				break;
			}
			if ( (*lpstr) == cspacing )
			{ // 是分隔符,记下
				(*lpstr) = L'\0';
				lpCompiler->push(nRow);
			}
			lpstr++;
			nRow++;
		}
		return 0;
	}
	return -1;
}

class QyParserPrivateT
{
public:
	QyParserPrivateT(){}

	STD_TString  str_; ///< 字符指针
	CharT  cspacing_; ///< 分隔符
	std::vector<short> rows_; ///< 参数点
};

// 分析类	
QyParserT::QyParserT()
{
	d_ptr_ = new QyParserPrivateT();
}

QyParserT::~QyParserT()
{
	Clear();
	qyDeleteM( d_ptr_ );
}

int  QyParserT::Handle( const CharT* value,bool bDefCB)
{
	return Handle( value , qyos_strlen(value) , bDefCB);
}

int  QyParserT::Handle(const CharT* lpszTxt,size_t len,bool bDefCB)
{
	Clear();
	if( len && lpszTxt )
	{
		d_ptr_->str_ = lpszTxt;
	}
	if ( bDefCB )
	{
		return HandleCB(NULL);
	}
	return 0;
}

int  QyParserT::HandleCB(QyPLineParseCallBackT pCB , CharT cspacing)
{
	if ( d_ptr_->str_.length() <= 0  )
		return -1;
	d_ptr_->cspacing_ = cspacing;
	return (pCB ? pCB(this,cspacing) : DefLineParseCallbackT(this,cspacing) );
}

const CharT* QyParserT::String(size_t nRow,int &len ,const CharT *lpszDefault)
{	
	if ( d_ptr_->str_.length() <= 0 ) 
		return lpszDefault;

	size_t size =  d_ptr_->rows_.size();
	if(nRow >= (size-1) ){
		return lpszDefault;
	}		
	len = 0;
	CharT* lpString = const_cast<CharT*>(d_ptr_->str_.c_str() );
	len  = d_ptr_->rows_.at(nRow+1);
	nRow = (nRow > 0 ) ?(d_ptr_->rows_.at(nRow)+1):0;
	len -= nRow;
	return (lpString + nRow );
}

int QyParserT::Int(size_t nRow,int nDefault)
{
    if (  d_ptr_->str_.length() <= 0) 
		return nDefault;
	size_t size =  d_ptr_->rows_.size();
	if(nRow >= size-1 ){
		return nDefault;
	}
	CharT* lpString = const_cast<CharT*>(d_ptr_->str_.c_str() );
	nRow = (nRow > 0 ) ? ( d_ptr_->rows_.at(nRow) + 1 ) : 0;
	return qyos_atoi(lpString+ nRow );
}

double QyParserT::Double(size_t nRow,double dDefault)
{
    if (  d_ptr_->str_.length() <= 0)
		return dDefault;
	size_t size =  d_ptr_->rows_.size();
	if(nRow >= (size-1) ) {
		return dDefault;
	}
	CharT* lpString = const_cast<CharT*>(d_ptr_->str_.c_str() );
	nRow = (nRow > 0 ) ?( d_ptr_->rows_.at(nRow)+1):0;
	return qyos_atof(lpString + nRow);
}

const CharT* QyParserT::c_str(void) 
{ 
	return d_ptr_->str_.c_str();
}

size_t QyParserT::Length(void) 
{ 
	return  d_ptr_->str_.length();
}

void QyParserT::Clear()
{
	// 还原
	CharT *lpstr = const_cast<CharT*>(c_str());
	size_t  size = d_ptr_->rows_.size();
	if( size > 2 && lpstr )
	{
		for ( size_t i = 1; i < size - 1 ; i++ )
		{
			*(lpstr + d_ptr_->rows_[i] ) = d_ptr_->cspacing_;
		}
	}
	d_ptr_->rows_.clear();
}


size_t  QyParserT::Counts(void) const { 
	return d_ptr_->rows_.size() -1; }

void QyParserT::push(short row) {
	d_ptr_->rows_.push_back(row);}

END_NAMESPACE(qy)
