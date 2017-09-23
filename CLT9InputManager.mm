//
//  CLT9InputManager.m
//  CarLife
//
//  Created by ZhangHengming on 24/10/2016.
//  Copyright © 2016  Zhang Hengming. All rights reserved.
//
#import "CLT9InputManager.h"
#import "T9WordObject.h"
#include "T9InputEngine.hpp"


//#define lazyLoadT9Result
vector<string> remainSearchIndexVector;
@interface CLT9InputManager ()

@property(nonatomic,strong) NSOperationQueue* operationQueue;
@property(nonatomic,strong) NSOperation*      prevOperation;

@end

@implementation CLT9InputManager

+ (instancetype)sharedInstance {
    static CLT9InputManager* instance;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[CLT9InputManager alloc] init];
    });
    return instance;
}

- (instancetype)init {
    self = [super init];
    self.operationQueue = [[NSOperationQueue alloc] init];
    NSString *dbPath = [[NSBundle bundleWithPath:[[NSBundle mainBundle] bundlePath]] pathForResource:@"keyboardLib.sqlite" ofType:nil];

    //NSString *dbPath = [[NSBundle bundleWithPath:[[NSBundle mainBundle] bundlePath] ] pathForResource:@"keyboardLib.sqlite" ofType:nil];

    T9InputEngine::getInstance()->initDB([dbPath UTF8String]);
    return self;
}
  
- (void)generateResultWithIndex:(NSString*)inputT9String completion:(CompletionBlock)completion {
    
    //    string inputTest("shen");
    //    vector<string> testResult = searchKeyboard(inputTest);
    //    for (int i = 0; i < testResult.size(); i++) {
    //        cout<<testResult[i]<<endl;
    //    }
    
    //先把之前的检索都取消，之前的检索没有意义
    [self.prevOperation cancel];
    [self.operationQueue cancelAllOperations];
        NSBlockOperation* blockOperation = [NSBlockOperation blockOperationWithBlock:^{
        if ([NSThread currentThread] == [NSThread mainThread]) {
            NSLog(@"居然是在主线程检索数据库，看看调用栈%@",[NSThread callStackSymbols]);
        }
        string inputString = [inputT9String UTF8String];
        T9InputEngine* inputEngine = T9InputEngine::getInstance();
        const char* inputStringArray = inputString.c_str();
        vector< vector<const char*> > result = inputEngine->parsePinyinFromT9(inputStringArray);
        
        vector<string> searchIndexVector;
        for (vector< vector<const char*> >::iterator it = result.begin(); it != result.end(); it++ ) {
            vector<const char*> currentRow = *it;
            if (searchIndexVector.size() != 0) {
                unsigned long times = currentRow.size();
                unsigned long  currentLength = searchIndexVector.size();
                searchIndexVector.reserve(times*searchIndexVector.size());
                times--;
                while (times--)
                {
                    for (int i = 0; i < currentLength; i++) {
                        //                    cout<<"copying...   "<<searchIndexVector[i]<<endl;
                        searchIndexVector.push_back(searchIndexVector[i]);
                    }
                }
                //穷举所有的拼音组合可能
                int currentIndex = 0;
                for (int j = 0; j < searchIndexVector.size(); j = (int)(j + currentLength) ) {
                    for (int temp = j; temp < j + currentLength ; temp++ ) {
                        string s = searchIndexVector[temp];
                        cout<<"left string is "<<s<<endl;
                        string appendingString = currentRow[currentIndex];
                        cout<<"right string is "<<currentRow[currentIndex]<<endl;
                        searchIndexVector[temp].append(currentRow[currentIndex]);
                    }
                    currentIndex ++;
                }
                
            } else {
                for (vector<const char*>::iterator iter = currentRow.begin(); iter != currentRow.end(); iter++) {
                    string temp(*iter);
                    searchIndexVector.push_back(temp);
                }
            }
        }
        remainSearchIndexVector.assign(searchIndexVector.begin(), searchIndexVector.end());
        NSMutableArray* resultArray = [NSMutableArray new];
        
        
#ifdef lazyLoadT9Result
        //先初始化给20个候选词，保证装满前两页
        while (resultArray.count <= 20 && remainSearchIndexVector.size() > 0) {
            vector<T9Word> finaleReuslt =  [self lateSearchIndex:remainSearchIndexVector];
            for (vector<T9Word>::iterator it = finaleReuslt.begin(); it!=finaleReuslt.end() ; it++) {
                T9WordObject *t9WordObject = [[T9WordObject alloc] initWithWord:];
                T9WordObject *t9WordObject = [[T9WordObject alloc] initWithWord:[NSString stringWithUTF8String:((string)((*it).word)).c_str()] frequent:(double)((*it).frequent)];
                [resultArray addObject:t9WordObject];
                //[resultArray addObject:[NSString stringWithUTF8String:((string)(*it)).c_str()]];
            }
        }
#else
        
        vector<T9Word>finalResult = [self lateSearchIndex:remainSearchIndexVector];
        for (vector<T9Word>::iterator it = finalResult.begin(); it!= finalResult.end() ; it++ ) {
            T9WordObject *t9WordObject = [[T9WordObject alloc] initWithWord:[NSString stringWithUTF8String:((string)((*it).word)).c_str()] frequent:(double)((*it).frequent)];
            [resultArray addObject:t9WordObject];
//            [resultArray addObject:[NSString stringWithUTF8String:((string)(*it)).c_str()]];
        }
        
#endif
        dispatch_async(dispatch_get_main_queue(), ^{
//      [self updateSearchResult:resultArray searchKey:self.searchKey];
            completion(resultArray);
        });
    
    
    
    
    
    
    
    
    }];
    
    
    self.prevOperation = blockOperation;
    [self.operationQueue addOperation:self.prevOperation];

}




/**
 分次检索词库，加载结果
 
 @param input 搜索索引，拼音组合
 @return 检索结果
 */
- (vector<T9Word>)lateSearchIndex:(vector<string>&)input
{
    //1.取input最后一个组合去检索词库
    //2.拿出结果，并且判断结果是否足够铺满一页
    //3.若结果不够铺满一页，则pop最后一个组合，继续拿最后一个组合去检索，直至铺满一页 or input为空。
    //4.当点击下一页按钮到最后一页时候，重复1~3
    
    T9InputEngine* t9Engine = T9InputEngine::getInstance();
    vector<T9Word> result;
#ifdef lazyLoadT9Result
    //保证返回结果20个
    while (input.size() > 0 && result.size() < 20) {
#else
        while (input.size()>0) {
#endif
            vector<T9Word>temp = t9Engine->queryWords(input[input.size()-1]);
            result.insert(result.end(),temp.begin(),temp.end());
            input.pop_back();
        }
        return result;
    }
    
@end
