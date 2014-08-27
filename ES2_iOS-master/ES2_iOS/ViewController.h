//
//  ViewController.h
//  AntiReplayIOS
//
//  Created by Kris Meeusen on 13/02/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#include <vector>
#include "cinder/Vector.h"

@interface ViewController : GLKViewController

//- (std::vector<ci::Vec2f>) NSSetToVector:(NSSet*) set;

- (void) restart;


@end
