﻿#include <realtime_srv/RealtimeServer.h>

#ifdef IS_LINUX

#include "Character.h"
#include "ExampleRedisCli.h"
#include "CharacterLuaBind.h"
#include "ExampleInputState.h"


using namespace realtime_srv;


class ExampleSrvForUe4Demo : noncopyable
{
public:
	ExampleSrvForUe4Demo( bool willDaemonizeOnLinux = false )
		:
		server_(
			std::bind( &ExampleSrvForUe4Demo::SpawnNewCharacter, this, _1 ),
			std::bind( &ExampleSrvForUe4Demo::MyInputState, this ),
			willDaemonizeOnLinux )
	{
		db_.Init( server_.GetEventLoop() );
	}

	InputState* MyInputState() { return new ExampleInputState(); }

	GameObjPtr SpawnNewCharacter( ClientProxyPtr& cliProxy )
	{
		// test hiredis
		db_.SaveNewPlayer( cliProxy->GetNetId(),
			cliProxy->GetPlayerName() );

		// test lua
		CharacterLuaBind clb;
		CharacterPtr newCharacter = clb.DoFile();
		newCharacter->SetPlayerId( cliProxy->GetNetId() );
		return  std::static_pointer_cast< GameObj >( newCharacter );
	}

	//	假设 : 
	// 	LatencyCmdIndex = 2;
	//	dropPacketChanceCmdIndex = 3;
	//	JitterCmdIndex = 4;
	//	那在Windows上,如果输入命令:   ./example_for_ue4_demo.exe 0 0.3 0.8 1
	//	则服务器将会模拟 : 
	//	-	模拟延迟为 0.3 : 当前延迟+300毫秒的延迟, 若当前延迟为30ms, 则模拟之后的延迟约为30+300=330左右
	//	-	模拟丢包率为 0.8 : 百分之八十的丢包率
	//	-	模拟网络抖动为 : 1为随机抖动, 0为不抖动
	void Run() { server_.SimulateRealWorldOnWin( 2, 3, 4 ); server_.Run(); }

private:

	RealtimeServer server_;
	ExampleRedisCli db_;
};



int main( int argc, const char** argv )
{
	RealtimeSrvHelper::SaveCommandLineArg( argc, argv );

	// 在Linux上, 加上一个命令行参数 1 即可变为守护进程,   
	// 如:	   ./rs_example_for_ue4_demo  1
	// 不加即为前台运行
	const uint8_t DaemonizeCmdIndexOnLinux = 1;
	bool willDaemonizeOnLinux = RealtimeSrvHelper::GetCommandLineArg(
		DaemonizeCmdIndexOnLinux ) == "1";

	ExampleSrvForUe4Demo exmaple_server( willDaemonizeOnLinux );
	exmaple_server.Run();

}

#endif // IS_LINUX