# USLanMatcher

- Ultra-Slim Lan Matcher is Unreal plugin supporting host matching server and notify server address to particiate.
- Based on Unreal’s `Subsystem`, You can make very simple LAN Matching server by including module.

## How it works?

- Server: If you host server, It opens socket to receive **Join Reqeust** from client. You can get the **Join Request**s. If acceptable, You can send **Join Ack** to client containing game server address.
- Client: After you host socket to receive response from server, You can send the **Join Request.**
If you receive **Join Ack** from server, that infromation will include game server address and you can just join to it.

# API Lists

# USLanMatcherHostServerSubsystem

- This class is the class to
    1. host/close server
    2. receive join request
    3. send join ack
- extends `UGameInstanceSubsystem`

**SOCKET_CONSTRUCT_RESULT HostServer(int32 DesiredPortNo = DEFAULT_SERVER_PORT_NO);**

- Host socket to receive response from client.
- If you host server, the server automatically generate random server ID(6 of capital letter)
- **DesiredPortNo:** Port number you want to host server. (default is 8889)
- **return:** SOCKET_CONSTRUCT_RESULT

**void CloseServer();**

- Close current Socket

**FString GetServerID();**

- **return:** generated ServerID

**uint32 GetServerIDHash();**

- **return:** this server uses Hash ID of string to validate packet. It returns the HashID.

**FString GetCurrentWorldIP(bool bWithPortNo = true);**

- **return:** Current World’s Address of game server where clients will join

**JOIN_REQ_RECV_RESULT TryReceiveJoinReq(JoinRequesterInfo& outSenderInfo);**

- **outSenderInfo**: If successful, return sender info want to participate.
- **return:** JOIN_REQ_RECV_RESULT

**JOIN_ACK_SEND_RESULT SendJoinAck(const JoinRequesterInfo& inDestInfo, const FString& inGameServerIP);**

- **inDestInfo:** Client’s info to send **Join Ack**
- **inGameServerIP:** game server address to enter

**void ProcessRequests(const FString& InGameServerURL);**

- Process all pending join requests.
- If only the packet is valid, send **Join Ack** to every clients without any additional condition.
- **inGameServerIP:** game server address to enter.

# USLanMatcherClientSubsystem

- This class is the class to
    1. host/close socket to receive **Join Ack** 
    2. send **Join Request**
    3. receive **Join Ack**
- extends `UGameInstanceSubsystem`

**SOCKET_CONSTRUCT_RESULT HostResponseRecvSocket(int32 DesiredPortNo);**

- Host socket to receive response from server.
- **DesiredPortNo:** Port number you want to host server. (default is 0 which is random)
- **return:** SOCKET_CONSTRUCT_RESULT

**void CloseClientSocket();**

- Close current Socket

**JOIN_REQ_SEND_RESULT SendJoinReq(const TCHAR* ServerID, int32 ServerPortNo, bool bIsBroadcast, FString TargetIPAddr);**

- send socket requesting to join server. (Default action is broadcast)
- **ServerID:** ServerID of Matching server.
- **ServerPortNo:** Session server port number. (Default is DEFAULT_SERVER_PORT_NO)
- **bIsBroadcast:** If true, broadcast request. In false, send join request to TargetIPAddr
- **TargetIPAddr:** IP Addr to send Join request. bIsBroadcast must be false. Not include Port No.

**JOIN_ACK_RECV_RESULT TryRecvJoinAck(FString& outGameServerAddr);**

- **outString**: If successful, return Game ServerAddr.
- **return:** JOIN_REQ_RECV_RESULT

# Contact

happy981024@naver.com