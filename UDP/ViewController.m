/*!
 @file ViewControl.m
 @author vigek
 @copyright 2017 vigek
 @version 17.10.13t;
 */

#import "ViewController.h"
#import "AsyncUdpSocket.h"
#import <ifaddrs.h>
#import <arpa/inet.h>
/*! the message you want send */
NSString *request;
/*! the message you receive */
NSString* result;
@interface ViewController ()<AsyncUdpSocketDelegate>
@property (weak, nonatomic) IBOutlet UITextField *sendMessage;
@property (weak, nonatomic) IBOutlet UITextField *getMessage;
-(IBAction)beginSend:(id)sender;
-(void)viewDidLoad;
-(void)didReceiveMemoryWarning;
-(void)MakeUDP;
-(NSString *)getIPAddress;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self MakeUDP];
  
}

/*!
 @brief This method is used to make udp request
 */
-(void)MakeUDP
{
   
    AsyncUdpSocket *socket=[[AsyncUdpSocket alloc]initWithDelegate:self];
    /*! open the local port */
    [socket localPort];
    /* set the timeout */
    NSTimeInterval timeout=1000;
    
    
    
    NSData *data=[NSData dataWithData:[request dataUsingEncoding:NSASCIIStringEncoding] ];
    
    UInt16 port=6000;
    
    NSError *error;
    
    /*! set the send broadcast */
    [socket enableBroadcast:YES error:&error];
    /*! 把得到的目标ip 最后的数字更换为255（意思是搜索全部的）*/
    NSArray *strArr=[[self getIPAddress] componentsSeparatedByString:@"."];
    NSMutableArray *muArr = [NSMutableArray arrayWithArray:strArr];
    [muArr replaceObjectAtIndex:(strArr.count-1) withObject:@"255"];
    /*! the target IP */
    NSString *finalStr = [muArr componentsJoinedByString:@"."];
    
    /* send request */
    BOOL _isOK = [socket sendData :data toHost:[NSString stringWithFormat:@"%@",finalStr] port:port withTimeout:timeout tag:1];
    if (_isOK) {
        /*! udp request successfully */
    }else{
        /*! udp request fail */
    }
    /*! begin receving thread */
    [socket receiveWithTimeout:1000 tag:0];
   
    NSLog(@"开始啦");
}

/*!
 @brief this method is used to receive message
 */
-(BOOL)onUdpSocket:(AsyncUdpSocket *)sock didReceiveData:(NSData *)data withTag:(long)tag fromHost:(NSString *)host port:(UInt16)port{
    
    
    
    result = [[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding];
    
    NSLog(@"%@",result);
    
    NSLog(@"%@",host);
    
    NSLog(@"收到啦");
    [_getMessage setText:result];
    
    return NO;
}

/*!
 @brief Receive message failed.
 */
-(void)onUdpSocket:(AsyncUdpSocket *)sock didNotReceiveDataWithTag:(long)tag dueToError:(NSError *)error{
    
    NSLog(@"没有收到啊 ");
}

/*!
 @brief send message failed.
 */
-(void)onUdpSocket:(AsyncUdpSocket *)sock didNotSendDataWithTag:(long)tag dueToError:(NSError *)error{
    
    NSLog(@"%@",error);
    
    NSLog(@"没有发送啊");
    
}

/*!
 @brief Begin to send message.
 */
-(void)onUdpSocket:(AsyncUdpSocket *)sock didSendDataWithTag:(long)tag{
    
    NSLog(@"发送啦");
}

/*!
 @brief Close the broadcast.
 */
-(void)onUdpSocketDidClose:(AsyncUdpSocket *)sock{
    
    NSLog(@"关闭啦");
}

#pragma mark 获取当前IP
- (NSString *)getIPAddress {
    NSString *address = @"error";
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = 0;
    /*! retrieve the current interfaces - returns 0 on success */
    success = getifaddrs(&interfaces);
    if (success == 0) {
        /*! Loop through linked list of interfaces */
        temp_addr = interfaces;
        while(temp_addr != NULL) {
            if(temp_addr->ifa_addr->sa_family == AF_INET) {
                /*! Check if interface is en0 which is the wifi connection on the iPhone */
                if([[NSString stringWithUTF8String:temp_addr->ifa_name] isEqualToString:@"en0"]) {
                    /*! Get NSString from C String */
                    address = [NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr)];
                }
            }
            temp_addr = temp_addr->ifa_next;
        }
    }
    /*!Free memory */
    freeifaddrs(interfaces);
    return address;
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
   
}

- (IBAction)beginSend:(id)sender {
    request=[_sendMessage text];
    [self MakeUDP];
    
}
@end
