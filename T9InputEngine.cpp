//
//  T9InputEngine.cpp
//  CarLife
//
//  Created by ZhangHengming on 23/09/2016.
//  Copyright © 2016  Zhang Hengming. All rights reserved.
//


#include "T9InputEngine.hpp"
#include <mutex>
#include <memory>
#include <stdlib.h>

#define useSeparator

static T9InputEngine* instance;
mutex mtx;
static mutex dataBaseMutex;

const char separator = '`';




#pragma mark - 类初始化、析构函数、构造函数
T9InputEngine* T9InputEngine::getInstance()
{
    if (instance == NULL)
    {
        mtx.lock();
        if (instance == NULL)
        {
            instance = new T9InputEngine();
        }
        mtx.unlock();
    }
    return instance;
}

void T9InputEngine::releaseInstance()
{
    delete instance;
    instance = NULL;
}


T9InputEngine::~T9InputEngine()
{
    sqlite3_close(this->db);
    this->db = NULL;
    this->currentCache.clear();
}

#pragma mark - 解析字符串相关

bool hasSeparator(char* input)
{
    size_t length = strlen(input);
    for (int i = 0;  i < length ; i++ )
    {
        if (input[i] == separator)
        {
            return true;
        }
    }
    return false;
}

const T9PY_IDX* T9InputEngine::resultIndex(char* inputstr, int length)
{
    const T9PY_IDX *cpHZ,*cpHZedge;
    int PY_num=0;
    
    if(*inputstr=='\0')
        return(0); //如果输入空字符返回0
    
    cpHZ = &(t9PY_index[0]);//取得查找表的首地址
    cpHZedge = t9PY_index + sizeof(t9PY_index)/sizeof(t9PY_index[0]);
    
    
    /*取得查找表的末地址 */
    //这里稳定以后可以考虑二分法优化By eric
    while(cpHZ < cpHZedge) /*如果没有搜索到末地址则继续 */
    {
        if(strcmp(inputstr,(*cpHZ).T9) == 0)
        {
            PY_num ++;
            break;
        }
        cpHZ++;
    }
    if (PY_num>0)
    {
        return cpHZ;
    }
    else
    {
        return NULL; //没有查到
    }
}

bool T9InputEngine::matchPinyin(char* input, vector<const char*>&resultVector)
{
    cout<<"检测"<<input<<"匹配"<<endl;
    if (strlen(input) == 0)
    {
        return false;
    }
    //    cout<<"检索用的Index"<<input<<endl;
    
    const T9PY_IDX* result = resultIndex(input, (int)strlen(input));
    if (result == NULL)
    {
        return false;
    }
    else
    {
        while (strcmp(result->T9,input) == 0) {
            string temp(result->PY);
            const char* pinyin = result->PY;
            resultVector.push_back(pinyin);
            result++;
        }
        cout<<input<<"匹配拼音成功"<<endl;
        return true;
    }
}

bool T9InputEngine::matchPinyin(char* input)
{
    cout<<"检测"<<input<<"匹配"<<endl;
    if (strlen(input) == 0)
    {
        return false;
    }
    //    cout<<"检索用的Index"<<input<<endl;
    
    const T9PY_IDX* result = resultIndex(input, (int)strlen(input));
    if (result == NULL)
    {
        return false;
    }
    else
    {
        while (strcmp(result->T9,input) == 0) {
            //            string temp(result->PY);
            //            const char* pinyin = result->PY;
            result++;
        }
        cout<<input<<"匹配拼音成功"<<endl;
        return true;
    }
}





bool T9InputEngine::isPinyin(const char* pinyin)
{
    bool ret = false;
    const T9PY_IDX *cpHZ,*cpHZedge;
    int PY_num=0;
    
    if(*pinyin=='\0')
        return(0); //如果输入空字符返回0
    
    cpHZ = &(t9PY_index[0]);//取得查找表的首地址
    cpHZedge = t9PY_index + sizeof(t9PY_index)/sizeof(t9PY_index[0]);//表的末地址
    
    
    /*取得查找表的末地址 */
    while(cpHZ < cpHZedge) /*如果没有搜索到末地址则继续 */
    {
        if(strcmp(pinyin,(*cpHZ).PY) == 0)
        {
            PY_num ++;
            break;
        }
        cpHZ++;
    }
    
    if (PY_num>0)
    {
        ret = true;
    }
    else
    {
        ret = false;//没有查到
    }
    return ret;
}

vector< vector<const char*> >T9InputEngine::parsePinyinFromT9(const char* input)
{
    cout<<"current t9 string"<<input<<endl;
    size_t length = strlen(input);
    vector<vector<const char*> > result;
    int i = 0;
    int j = 0;
    while( i < length )
    {
        j = i + 1;
        vector<const char*>temp;
        while (j <= length)
        {
            char* index;
            //在这里判断一下最后一位是不是分词符号，是的话取分词符号为间隔探测拼音，移动指针，不再进行后面的操作
#ifdef useSeparator
            if (input[j] == separator)
            {
                index = new char[j-i];
                index[j-i-1] = 0;
                strncpy(index, input+i,j-i);
                matchPinyin(index, temp);
                i = ++j;
                break;
            }
            else
            {
                index = new char[j-i + 1];
                index[j-i] = 0;
                strncpy(index, input+i, j-i );
                //                if (j < length) {
                //                    continue;
                //                } else {
                //                    break;
                //                }
            }
#else
            index = new char[j-i + 1];
            index[j-i] = 0;
            strncpy(index, input+i, j-i );
#endif
            cout<<"check t9 match pinyin "<<index<<endl;
            if (matchPinyin(index, temp))
            {
                cout<<index<<"match"<<endl;
                delete[] index;
                index = NULL;
                j++;
                if (j > length)
                {
                    i = j;
                    break;
                }
            }
            else
            {
                //当前匹配不上拼音的话有两种情况（要么超过了最长匹配，要么包含了一个分隔符（其实理论上也不该有这种情况吧））
                if (strcmp(index, &separator)==0)
                {
                    i = j + 1;
                    delete[] index;
                    index = NULL;
                    break;
                }
                
                i = j - 1;
                j = j + 1;//这一行的作用有待研究
                delete[] index;
                index = NULL;
                break;
            }
            //每个vector的最后一个是最长匹配子串
        }
        result.push_back(temp);
    }
    return result;
}



/**
 带分隔符的解析拼音接口
 
 @param input 输入的带解析符的字符串
 @return 返回所有拼音的组合。二维矩阵的每一行都代表一串字符串对应的拼音组合
 */
vector< vector<const char*> >  T9InputEngine::parsePinyinFromT9WithSeparator(const char* input)
{
    size_t length = strlen(input);
    int i =0, j = 0;
    vector< vector< const char* > > result;
    //todo: 通过分隔符检索
    while (i < length )
    {
        vector<const char*> temp;
        while (input[j] != separator)
        {
            j++;
        }
        //此时J位于分隔符上
        char* index = new char[j-i];
        strncpy( index, input+i , j-i );
        matchPinyin(index,temp);
        i = ++j;
        j = j + 1;
        result.push_back(temp);
    }
    return result;
}

stack<string> T9InputEngine::generateIndexWithSeparator(const char* input)
{
    stack<string> result;
    shared_ptr<IndexNode> root(new IndexNode);
    size_t length = strlen(input);
    int i = 0;
    int j = i;
    while (i < length)
    {
        //        int j = i + 1;
        j++;
        char* tempIndex = new char[j-i];
        strncpy(tempIndex,input+i,j-i);
        vector<const char*>tempString;
        if (matchPinyin(tempIndex))
        {
            IndexNode* tempNode = new IndexNode;
            tempNode->currentString = tempIndex;
            tempNode->remainedString = new char[length - j + i];
            strncpy(tempNode->remainedString, input+(j-i), length-(j-i));
            root->subNodes.push_back(tempNode);
        }
        else
        {
            break;
        }
    }
    return result;
}

void T9InputEngine::generateSubNodes(IndexNode* root)
{
    char* remainedString = root->remainedString;
    size_t length = strlen(remainedString);
    if (length <= 0)
    {
        return ;
    }
    for (int i = 1; i <= length; i++ )
    {
        char* index = new char[i+i];
        memset(index, 0, i+1);
        strncpy(index, remainedString, i);
        //最长匹配之前，将所有匹配到的字符串放入子树中
        IndexNode* tempNode = new IndexNode;
        if (matchPinyin(index,tempNode->currentMatchedPinyin))
        {
            tempNode->currentString = index;
            tempNode->remainedString = new char[length - i + 1];
            memset(tempNode->remainedString, 0, length-i+1);
            strncpy(tempNode->remainedString, remainedString+i, length-i);
            size_t remainedLength = strlen(tempNode->remainedString);
            if (remainedLength > 0)
            {
                generateSubNodes(tempNode);
            }
            root->subNodes.push_back(tempNode);
        }
        else
        {
            delete tempNode;
            tempNode = NULL;
            break;
        }
    }
}

IndexNode* T9InputEngine::testParseInterface(char* input)
{
    IndexNode* root = new IndexNode;
    root->remainedString = input;
    generateSubNodes(root);
    return root;
}


IndexNode* T9InputEngine::parseT9String(char* input)
{
#pragma warning( push )
#pragma warning( disable : 4101)
    return this->testParseInterface(input);
#pragma warning( pop )
}


//vector<string> T9InputEngine::parseTreeIntoPinyin(IndexNode *tree) {
//    vector<string> result;
//
//    size_t subNodeSize = 0;
//    if ( tree->subNodes.size() ) {
//        subNodeSize = tree->subNodes.size();
//    };
//
//    for (int i = (int) (subNodeSize - 1); i > 0; i-- ) {
//        vector<string> tempResult =
//    }
//
//    return result;
//}

void T9InputEngine::recursiveTraversal(IndexNode* node)
{
    generateSubNodes(node);
    if (node->subNodes.size() > 0 )
    {
        for (int i = 0; i < node->subNodes.size(); i++)
        {
            recursiveTraversal(node->subNodes[i]);
        }
    }
}

void T9InputEngine::printTree(IndexNode* root ,vector<char*>& cache, vector< vector<char *>>& result)
{
    if (NULL == root) {
        return ;
    }
    
    if (NULL != root->currentString/* && strlen(root->currentString) > 0*/ ) {
        cache.push_back(root->currentString);
    }
    
    if (strlen(root->remainedString) == 0) {
        
    } else {
        for (int i = (int)root->subNodes.size() - 1; i >= 0; i-- ) {
            printTree(root->subNodes[i], cache, result);
        }
    }
    
    if ( root->subNodes.size() == 0 ) {
        vector<char *> temp;
        for (vector<char*>::iterator it = cache.begin(); it != cache.end(); it++ ) {
            temp.push_back(*it);
        }
        
        cache.pop_back();
        result.push_back(temp);
        //        cache.pop_back();
        return ;
    }
    cache.pop_back();
    return ;
}


#pragma mark - 词库检索相关

bool T9InputEngine::initDB(const char* DBPath)
{
    if (sqlite3_open_v2(DBPath, &db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }
    return true;
}

vector<T9Word> T9InputEngine::queryWords(string pinyin)
{
    printf("搜索用的index为%s\n",pinyin.c_str());
    vector<T9Word> resultVector;
    if (this->db == NULL)
    {
        return resultVector;
    }
    sqlite3_stmt* statement;
    string queryStatement = "select * from relate where pinyinOfAllWord = '";
    queryStatement = queryStatement + pinyin + "'";
    
    if ( sqlite3_prepare(this->db, queryStatement.c_str(), -1, &statement, NULL) == SQLITE_OK )
    {
        while (sqlite3_step(statement) == SQLITE_ROW )
        {
            string leftWord((char*)sqlite3_column_text(statement, 1));
            string rightWord((char*)sqlite3_column_text(statement, 2));
            double frequent = atof((char*)sqlite3_column_text(statement, 9));
            cout<<"查询拼音:"<<pinyin<<" word: "<<leftWord + rightWord<<endl;
            resultVector.push_back({leftWord + rightWord,frequent});
        }
    }
    
    if (resultVector.size() == 0)
    {
        if (pinyin.length() >= 1 && isPinyin(pinyin.c_str()))
        {
            string querySingleCharStatement = "select * from word where code = '" + pinyin + "'";
            if ( sqlite3_prepare(this->db, querySingleCharStatement.c_str(), -1, &statement, NULL) == SQLITE_OK )
            {
                while (sqlite3_step(statement) == SQLITE_ROW )
                {
                    string temp((char*)sqlite3_column_text(statement, 2));
                    double frequent = atof((char*)sqlite3_column_text(statement, 3));
                    cout<<"查询拼音:"<<pinyin<<" word: "<<temp<<endl;
                    resultVector.push_back({temp,frequent});
                }
            }
        }
    }
    sqlite3_finalize(statement);
    return resultVector;
}


#pragma mark - test method


