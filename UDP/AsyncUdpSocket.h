/*!
@file AsyncUdpSocket.h
@brief This is the header where some socket method is contained.
@author vigek
@copyright 2017 vigek
@version 17.10.13
*/

#import <Foundation/Foundation.h>
@class AsyncSendPacket;
@class AsyncReceivePacket;

extern NSString *const AsyncUdpSocketException;
extern NSString *const AsyncUdpSocketErrorDomain;
/*!
 @enum AsyncUdpSocketError
 @brief  A enum about the socket error.
 @field AsyncUdpSocketBadParameter  Used if given a bad parameter (such as an improper address).
 @field AsyncUdpSocketIPv4Unavailable Used if you bind/connect using IPv6 only
 @field AsyncUdpSocketIPv6Unavailable Used if you bind/connect using IPv4 only (or iPhone)
*/
 enum AsyncUdpSocketError
{
	AsyncUdpSocketCFSocketError = kCFSocketError,
	AsyncUdpSocketNoError = 0,
	AsyncUdpSocketBadParameter,
	AsyncUdpSocketIPv4Unavailable,
	AsyncUdpSocketIPv6Unavailable,
	AsyncUdpSocketSendTimeoutError,
	AsyncUdpSocketReceiveTimeoutError
};
typedef enum AsyncUdpSocketError AsyncUdpSocketError;

@interface AsyncUdpSocket : NSObject
{
 @public
    /*! IPv4 socket */
	CFSocketRef theSocket4;
    /*! IPv6 socket */
	CFSocketRef theSocket6;
	/*!  For theSocket4 */
	CFRunLoopSourceRef theSource4;
    /*! For theSocket6 */
	CFRunLoopSourceRef theSource6;
	CFRunLoopRef theRunLoop;
	CFSocketContext theContext;
	NSArray *theRunLoopModes;
	
	NSMutableArray *theSendQueue;
	AsyncSendPacket *theCurrentSend;
	NSTimer *theSendTimer;
	
	NSMutableArray *theReceiveQueue;
	AsyncReceivePacket *theCurrentReceive;
	NSTimer *theReceiveTimer;
	
	id theDelegate;
	UInt16 theFlags;
	
	long theUserData;
	
	NSString *cachedLocalHost;
	UInt16 cachedLocalPort;
	
	NSString *cachedConnectedHost;
	UInt16 cachedConnectedPort;
	
	UInt32 maxReceiveBufferSize;
}

/*!
 @brief Creates new instances of AsyncUdpSocket.
*/
- (id)init;
- (id)initWithDelegate:(id)delegate;
- (id)initWithDelegate:(id)delegate userData:(long)userData;

/*!
 @brief Creates new instances of AsyncUdpSocket that support only IPv4 or IPv6.
 @discussion The other init methods will support both, unless specifically binded or connected to one          protocol.
     If you know you'll only be using one protocol, these init methods may be a bit more efficient.
*/
- (id)initIPv4;
- (id)initIPv6;

- (id)delegate;
- (void)setDelegate:(id)delegate;

- (long)userData;
- (void)setUserData:(long)userData;

/*!
 @brief Returns the local address info for the socket.
 @tip Address info may not be available until after the socket has been bind'ed,
   or until after data has been sent.
*/
- (NSString *)localHost;
- (UInt16)localPort;

/*!
 @brief Returns the remote address info for the socket.
 @tip Since UDP is connectionless by design, connected address info will not be available unless the socket is explicitly connected to a remote host/port.
*/
- (NSString *)connectedHost;
- (UInt16)connectedPort;

/*!
 @brief Returns whether or not this socket has been connected to a single host.
 @discussion By design, UDP is a connectionless protocol, and connecting is not needed.
       If connected, the socket will only be able to send/receive data to/from the connected host.
*/
- (BOOL)isConnected;

/*!
 @brief  Returns whether or not this socket has been closed.
 @discussion The only way a socket can be closed is if you explicitly call one of the close methods.
*/
- (BOOL)isClosed;

/*!
 @brief  Returns whether or not this socket supports IPv4.
 @discussion By default this will be true, unless the socket is specifically initialized as IPv6 only,
     or is binded or connected to an IPv6 address.
*/
- (BOOL)isIPv4;

/*!
 @brief Returns whether or not this socket supports IPv6.
 @discusssion By default this will be true, unless the socket is specifically initialized as IPv4 only,
             or is binded or connected to an IPv4 address.This method will also return false on platforms that do not support IPv6.
 @tip Note: The iPhone does not currently support IPv6.
*/
- (BOOL)isIPv6;

/*!
 @brief Returns the mtu of the socket.If unknown, returns zero.
 @discussion Sending data larger than this may result in an error.This is an advanced topic, and one should understand the wide range of mtu's on networks and the internet.
     Therefore this method is only for reference and may be of little use in many situations.
*/
- (unsigned int)maximumTransmissionUnit;


/*!
 @brief Binds the UDP socket to the given port and optional address.
 @discussion Binding should be done for server sockets that receive data prior to sending it.
    Client sockets can skip binding,as the OS will automatically assign the socket an available port when it starts sending data.
     You cannot bind a socket after its been connected.
     You can only bind a socket once.
     You can still connect a socket (if desired) after binding.
     On success, returns YES.
     Otherwise returns NO, and sets errPtr. If you don't care about the error, you can pass nil for errPtr.
*/
- (BOOL)bindToPort:(UInt16)port error:(NSError **)errPtr;
- (BOOL)bindToAddress:(NSString *)localAddr port:(UInt16)port error:(NSError **)errPtr;

/*!
 @brief Connects the UDP socket to the given host and port.
 @discussion  By design, UDP is a connectionless protocol, and connecting is not needed.
     Choosing to connect to a specific host/port has the following effect:
     You will only be able to send data to the connected host/port.
     You will only be able to receive data from the connected host/port.
     You will receive ICMP messages that come from the connected host/port, such as "connection refused".
     Connecting a UDP socket does not result in any communication on the socket.
     It simply changes the internal state of the socket.
     You cannot bind a socket after its been connected.
     You can only connect a socket once.
     On success, returns YES.
     Otherwise returns NO, and sets errPtr. If you don't care about the error, you can pass nil for errPtr.
*/
- (BOOL)connectToHost:(NSString *)host onPort:(UInt16)port error:(NSError **)errPtr;
- (BOOL)connectToAddress:(NSData *)remoteAddr error:(NSError **)errPtr;

/*!
 @brief Join multicast group.
       Group should be an IP address (eg @"225.228.0.1")
*/
- (BOOL)joinMulticastGroup:(NSString *)group error:(NSError **)errPtr;
- (BOOL)joinMulticastGroup:(NSString *)group withAddress:(NSString *)interface error:(NSError **)errPtr;

/*!
 @brief By default, the underlying socket in the OS will not allow you to send broadcast messages.
 @discussion In order to send broadcast messages, you need to enable this functionality in the socket.
    A broadcast is a UDP message to addresses like "192.168.255.255" or "255.255.255.255" that is delivered to every host on the network.
    The reason this is generally disabled by default is to prevent accidental broadcast messages from flooding the network.
*/
- (BOOL)enableBroadcast:(BOOL)flag error:(NSError **)errPtr;

/*!
 @brief Asynchronously sends the given data, with the given timeout and tag.
 @discussion This method may only be used with a connected socket.
    If data is nil or zero-length, this method does nothing and immediately returns NO.
    If the socket is not connected, this method does nothing and immediately returns NO.
*/
- (BOOL)sendData:(NSData *)data withTimeout:(NSTimeInterval)timeout tag:(long)tag;

/*!
 @brief Asynchronously sends the given data, with the given timeout and tag, to the given host and port.
 @discussion This method cannot be used with a connected socket.
   If data is nil or zero-length, this method does nothing and immediately returns NO.
   If the socket is connected, this method does nothing and immediately returns NO.
   If unable to resolve host to a valid IPv4 or IPv6 address, this method returns NO.
*/
- (BOOL)sendData:(NSData *)data toHost:(NSString *)host port:(UInt16)port withTimeout:(NSTimeInterval)timeout tag:(long)tag;

/*!
 @brief Asynchronously sends the given data, with the given timeout and tag, to the given address.
 @discussion This method cannot be used with a connected socket.
    If data is nil or zero-length, this method does nothing and immediately returns NO.
    If the socket is connected, this method does nothing and immediately returns NO.
*/
- (BOOL)sendData:(NSData *)data toAddress:(NSData *)remoteAddr withTimeout:(NSTimeInterval)timeout tag:(long)tag;

/*!
 @brief Asynchronously receives a single datagram packet.
 @discussion If the receive succeeds, the onUdpSocket:didReceiveData:fromHost:port:tag delegate method will be called.
     Otherwise, a timeout will occur, and the onUdpSocket:didNotReceiveDataWithTag: delegate method will be called.
*/
- (void)receiveWithTimeout:(NSTimeInterval)timeout tag:(long)tag;

/*!
 @brief Closes the socket immediately. Any pending send or receive operations are dropped.
*/
- (void)close;

/*!
 @brief Closes after all pending send operations have completed.
 @discussion After calling this, the sendData: and receive: methods will do nothing.
    In other words, you won't be able to add any more send or receive operations to the queue.
    The socket will close even if there are still pending receive operations.
*/
- (void)closeAfterSending;

/*!
 @brief Closes after all pending receive operations have completed.
 @discussion After calling this, the sendData: and receive: methods will do nothing.
    In other words, you won't be able to add any more send or receive operations to the queue.
    The socket will close even if there are still pending send operations.
*/
- (void)closeAfterReceiving;

/*!
 @brief Closes after all pending send and receive operations have completed.
 @discussion After calling this, the sendData: and receive: methods will do nothing.
     In other words, you won't be able to add any more send or receive operations to the queue.
*/
- (void)closeAfterSendingAndReceiving;

/*!
 @brief Gets/Sets the maximum size of the buffer that will be allocated for receive operations.
 @discussion  The default size is 9216 bytes.
   The theoretical maximum size of any IPv4 UDP packet is UINT16_MAX = 65535.
   The theoretical maximum size of any IPv6 UDP packet is UINT32_MAX = 4294967295.
   In practice, however, the size of UDP packets will be much smaller.
   Indeed most protocols will send and receive packets of only a few bytes,or will set a limit on the size of packets to prevent fragmentation in the IP layer.
   If you set the buffer size too small, the sockets API in the OS will silently discard any extra data, and you will not be notified of the error.
*/
- (UInt32)maxReceiveBufferSize;
- (void)setMaxReceiveBufferSize:(UInt32)max;

/*!
 @brief When you create an AsyncUdpSocket, it is added to the runloop of the current thread.
       So it is easiest to simply create the socket on the thread you intend to use it.
 @discussion You need to move the socket to a separate thread at a later time, this
       method may be used to accomplish the task.
       This method must be called from the thread/runloop the socket is currently running on.
 @tip After calling this method, all further method calls to this object should be done from the given runloop.
     Also, all delegate calls will be sent on the given runloop.
*/
- (BOOL)moveToRunLoop:(NSRunLoop *)runLoop;

/*!
 @brief Allows you to configure which run loop modes the socket uses.The default set of run loop modes is NSDefaultRunLoopMode.
 @discussion If you'd like your socket to continue operation during other modes, you may want to add modes such as NSModalPanelRunLoopMode or NSEventTrackingRunLoopMode. Or you may simply want to use NSRunLoopCommonModes.
 @tip Note: NSRunLoopCommonModes is defined in 10.5. For previous versions one can use kCFRunLoopCommonModes.
*/
- (BOOL)setRunLoopModes:(NSArray *)runLoopModes;

/*!
 @briefReturns the current run loop modes the AsyncSocket instance is operating in.
    The default set of run loop modes is NSDefaultRunLoopMode.
*/
- (NSArray *)runLoopModes;
-(BOOL)onUdpSocket:(AsyncUdpSocket *)sock didReceiveData:(NSData *)data withTag:(long)tag fromHost:(NSString *)host port:(UInt16)port;
@end



#pragma mark-
/*!
 @protocol AsyncUdpSocketDelegate
*/
@protocol AsyncUdpSocketDelegate
@optional

/*!
 @brief Called when the datagram with the given tag has been sent.
*/
- (void)onUdpSocket:(AsyncUdpSocket *)sock didSendDataWithTag:(long)tag;

/*!
 @brief Called if an error occurs while trying to send a datagram.
 @discussion This could be due to a timeout, or something more serious such as the data being too large to fit in a sigle packet.
*/
- (void)onUdpSocket:(AsyncUdpSocket *)sock didNotSendDataWithTag:(long)tag dueToError:(NSError *)error;

/*!
 @brief Called if the message has been received successully.
*/
-(BOOL)onUdpSocket:(AsyncUdpSocket *)sock didReceiveData:(NSData *)data withTag:(long)tag fromHost:(NSString *)host port:(UInt16)port;


/*!
 @brief Called if an error occurs while trying to receive a requested datagram.
 @discussion This is generally due to a timeout, but could potentially be something else if some kind of OS error occurred.
 */
- (void)onUdpSocket:(AsyncUdpSocket *)sock didNotReceiveDataWithTag:(long)tag dueToError:(NSError *)error;


/*!
 @brief Called when the socket is closed.
 @discussion A socket is only closed if you explicitly call one of the close methods.
*/
- (void)onUdpSocketDidClose:(AsyncUdpSocket *)sock;

@end
