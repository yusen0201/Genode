/*
 * \brief  Loader service
 * \author Norman Feske
 * \date   2012-04-17
 */

/*
 * Copyright (C) 2010-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <monitor.h>
#include <timer_session/connection.h>
#include <failsafe_session/connection.h> 

#include <trace/timestamp.h>
#include <base/printf.h>

int main()
{
	using namespace Genode;
	
	Timer::Connection timer;
	size_t size=1024*1024;
	enum { STACK_SIZE = 8*1024 };
	static Cap_connection cap;
	static Rpc_entrypoint ep(&cap, STACK_SIZE, "comper_ep");

	static Signal_receiver sig_rec;
	Signal_context sig_ctx;
	Signal_context sig_ctx_srv;

	static Failsafe::Connection srv_monitor(1024*1024);
	srv_monitor.fault_sigh(sig_rec.manage(&sig_ctx_srv));

	static Failsafe::Session_component red_comp(size, *env()->ram_session(), cap);
	static Failsafe::Session_component comp(size, *env()->ram_session(), cap);

	comp.fault_sigh(sig_rec.manage(&sig_ctx));
	comp.start("hello_client", "", Native_pd_args());
	comp.session_request_unlock();
	red_comp.start("red_client", "red", Native_pd_args()); //block thread

	//comp_component->child_fault_sigh(sig_rec.manage(&sig_ctx));

	//comp.block_for_announcement();
	//env()->parent()->announce("Hello", comp.hello_root_cap());
	

	/*****************************************************
	*****            start redundancy		 *****
	*****************************************************/
	


	Genode::Signal s = sig_rec.wait_for_signal();
	
	if (s.num() && s.context() == &sig_ctx) {
		PLOG("got exception for child in client monitor");
		red_comp.red_start();
		//sig_rec.dissolve(&sig_ctx);
	} 
	else if (s.num() && s.context() == &sig_ctx_srv) {
		Trace::Timestamp a = Trace::timestamp();
		printf("fault detected in monitor: %d \n", a);
		comp.child_destroy();

		/* redundancy */
		PLOG("got signal from server monitor");
		//red_comp.red_start(); //block thread	
		red_comp.session_request_unlock();
		PLOG("redundancy started");
		
		/* recreate a child */
		//PDBG("going to recreate child");
		//static Failsafe::Session_component comp(size, *env()->ram_session(), cap);
		//comp.start("red_client", "rechild", Native_pd_args());
		//comp.session_request_unlock();
	
		//sig_rec.dissolve(&sig_ctx_srv);
	} 
	else {
		PERR("got unexpected signal while waiting for child");
		class Unexpected_signal { };
		throw Unexpected_signal();
	}
	sleep_forever();
	return 0;
}
