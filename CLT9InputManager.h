//
//  CLT9InputManager.h
//  CarLife
//
//  Created by ZhangHengming on 24/10/2016.
//  Copyright © 2016 Zhang Hengming. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef  void(^CompletionBlock)(NSArray* result);

@interface CLT9InputManager : NSObject

+ (instancetype)sharedInstance;



/**
 根据输入的索引（T9字符串）来检索。检索过程是异步进行的，完成通过completion回调，回调是在主线程执行的。

 @param index 数字的T9字符串
 @param completion 搜索结果回调，传入所有符合的NSString
 */
- (void)generateResultWithIndex:(NSString*)index completion:(CompletionBlock)completion;

@end
