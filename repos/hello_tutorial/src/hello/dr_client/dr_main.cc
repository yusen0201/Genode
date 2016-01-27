/*
 * \brief  Test client for the Hello RPC interface
 * \author Björn Döbel
 * \date   2008-03-20
 */

/*
 * Copyright (C) 2008-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/env.h>
//#include <base/thread.h>
#include <hello_session/client.h>
#include <hello_session/connection.h>
#include <failsafe_session/connection.h> //loader connection

#include <timer_session/connection.h>

using namespace Genode;

//struct Update : Genode::Thread<4096>
//{
//	bool signal = false;
//  	Hello::Session_client &h;
//
//	void entry()
//	{
//		static Signal_receiver sig_rec;
//		Signal_context sig_ctx;
//		static Loader::Connection load(1024*1024);
//		load.fault_sigh(sig_rec.manage(&sig_ctx));
//		Genode::Signal s = sig_rec.wait_for_signal();
//	
//			PLOG("in thread");
//		if (s.num() && s.context() == &sig_ctx) {
//			PLOG("got exception for child in thread");
//			Capability<Hello::Session> p_cap =
//    				env()->parent()->session<Hello::Session>("foo, ram_quota=4K");
//
//  			Hello::Session_client p(p_cap);
//			h = p;
//			signal = true;
//			PLOG("signal = true");
//		} else {
//			PERR("got unexpected signal while waiting for child");
//			class Unexpected_signal { };
//			throw Unexpected_signal();
//		}
//		
//		sig_rec.dissolve(&sig_ctx);
//	}
//
//	Update(Hello::Session_client &hi)
//	:
//		Thread("update"), h(hi)
//	{
//		start();
//	}
//};


int main(void)
{
	Timer::Connection timer;

	static Signal_receiver sig_rec;
	Signal_context sig_ctx;
	PDBG("create loader session");
	static Loader::Connection load(1024*1024);
	load.fault_sigh(sig_rec.manage(&sig_ctx));
	PDBG("connect to hello");
	Capability<Hello::Session> h_cap =
    		env()->parent()->session<Hello::Session>("foo, ram_quota=4K");

  	Hello::Session_client h(h_cap);

	while (1) {

		h.say_hello();
		int foo = h.add(2, 5);
		PDBG("Added 2 + 5 = %d", foo);
		timer.msleep(1000);
	
		Genode::Signal s = sig_rec.wait_for_signal();

		if (s.num() && s.context() == &sig_ctx) {
			Capability<Hello::Session> p_cap =
    				env()->parent()->session<Hello::Session>("foo, ram_quota=4K");

  			Hello::Session_client p(p_cap);
			h = p;
		} 	
	else {
		PERR("got unexpected signal while waiting for child");
		class Unexpected_signal { };
		throw Unexpected_signal();
		}
	}
	return 0;
}
