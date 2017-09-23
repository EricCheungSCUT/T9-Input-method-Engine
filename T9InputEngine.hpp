//
//  T9InputEngine.hpp
//  CarLife
//
//  Created by ZhangHengming on 23/09/2016.
//  Copyright © 2016  Zhang Hengming. All rights reserved.
//

#ifndef T9InputEngine_hpp
#define T9InputEngine_hpp

#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include "T9PinyinIndex.h"
//若报错undefine symbols sqlite3_close...等，在xcode中Link Binary with Libraries  libsqlite3.tbd这个库
#include "sqlite3.h"
#include <stack>
using namespace std;


struct IndexNode
{
    char* remainedString;//剩余的字符串
    char* currentString;//当前的字符串
    vector< const char* > currentMatchedPinyin;//当前字符串对应的拼音
    vector<IndexNode*> subNodes;
};

struct T9Word
{
    string word;     //汉字/词
    double frequent; //频率
};

class T9InputEngine {
public:
    static  T9InputEngine* getInstance();
    ~T9InputEngine();
    static  void releaseInstance();
    /**
     解析输入的T9字符串
     
     @param input T9字符串，为数字
     
     @return 解析出的所有可能的拼音组合
     */
    vector<vector<const char*> > parsePinyinFromT9(const char* input);
    
    
    bool initDB(const char* dbpath);
    
    /**
     在词库里检索拼音对应的词
     
     @param pinyin 拼音字符串，like "yangguang", "shennanhuayuan"
     
     @return 结果集合
     */
    vector<T9Word> queryWords(string pinyin);
    
    /**
     只解析带分隔符的T9字符串接口
     
     @param input 带分隔符的T9字符串，并且每个词都是用分隔符分隔的
     @return 所有可能的拼音vector
     */
    vector< vector<const char*> >  parsePinyinFromT9WithSeparator(const char* input);
    
    
    
    /**
     打印整棵树
     
     @param root 根节点
     */
    void printTree(IndexNode* root ,vector<char*>& cache, vector< vector<char *>>& result);
    
    
    
    /**
     测试接口
     
     @param input 输入的t9字符串
     @return 解析出来的树
     */
    [[deprecated("测试接口")]]
    IndexNode* testParseInterface(char* input);
    
    
    /**
     将T9数字字符串切分，转化成树（中间形态）
     
     @param input T9数字字符串
     @return 解析出来的树
     */
    IndexNode* parseT9String(char* input);
    
//    vector<string> parseTreeIntoPinyin(IndexNode *tree);
    
    /**
     解析生成的拼音树
     
     @param root 拼音树根节点
     @return 所有拼音组合索引
     */
    vector<const char *>generateIndexFromeTree(IndexNode* root);
    
    
    
private:
    /**
     检查输入的T9字符串是否可以被解析成拼音，并且存放解析结果到resultVector中
     
     @param input        数字形式的T9字符串
     @param resultVector 存放解析的所有可能结果的vector
     
     @return 是否能被解析为拼音
     */
    bool matchPinyin(char* input,vector<const char*>& resultVector);
    
    
    
    /**
     单纯检测T9字符串是否能匹配拼音
     
     @param input T9数字字符串
     @return match or not
     */
    bool matchPinyin(char* input);
    
    
    
    /**
     查找某个T9输入 匹配的首个T9PY_INDEX指针
     
     @param inputstr 输入的T9字符串
     @param length   长度
     
     @return 首个匹配的T9PY_INDEX指针
     */
    const T9PY_IDX* resultIndex(char* inputstr,int length);
    
    /**
     某个英文字符串(例如"ai,abc,bai")是否为单个字的拼音
     
     @param pinyin 表示单个中文字符的拼音
     
     @return 是否为单个字符的拼音
     */
    bool isPinyin(const char* pinyin);
    
    
    
    /**
     生成所有带分隔符的索引，通过无分隔符的T9字符串
     
     @param input 输入的无分隔符的T9字符串
     @return 存放索引的栈
     */
    stack <string> generateIndexWithSeparator(const char* input);
    
    
    /**
     解析当前节点里剩余的字符串，将其
     
     @param root 根节点
     */
    void generateSubNodes(IndexNode* root);
    
    
    void recursiveTraversal(IndexNode* node);
    
protected:
    vector<string> currentCache;
    sqlite3* db;
};

#endif /* T9InputEngine_hpp */
