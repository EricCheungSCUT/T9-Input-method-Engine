//
//  T9WordObject.h
//  KeyBoardProject
//
//  Created by vinhankxiao on 2017/5/4.
//  Copyright © 2017年 张恒铭. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface T9WordObject : NSObject

@property (nonatomic, strong) NSString *word;
@property (nonatomic, assign) double frequent;

- (instancetype)initWithWord:(NSString *)word frequent:(double)frequent;

@end
