//
//  T9WordObject.m
//  KeyBoardProject
//
//  Created by vinhankxiao on 2017/5/4.
//  Copyright © 2017年 张恒铭. All rights reserved.
//

#import "T9WordObject.h"

@implementation T9WordObject

- (instancetype)initWithWord:(NSString *)word frequent:(double)frequent
{
    self = [super init];
    self.word = word;
    self.frequent = frequent;
    return self;
}

- (NSComparisonResult)compare:(T9WordObject *)otherObject {
    return [@(otherObject.frequent) compare:@(self.frequent)];
}

@end
