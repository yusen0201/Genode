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

#include <base/printf.h>
#include <trace/timestamp.h>

int main()
{
	using namespace Genode;
	
	Timer::Connection timer;
	size_t size=1024*1024;
	enum { STACK_SIZE = 8*1024 };
	static Cap_connection cap;
	static Rpc_entrypoint ep(&cap, STACK_SIZE, "loader_ep");
	static Signal_receiver sig_rec;
	Signal_context sig_ctx;

	static Failsafe::Session_component red_comp(size, *env()->ram_session(), cap);
	static Failsafe::Session_component comp(size, *env()->ram_session(), cap);

	comp.child_fault_sigh(sig_rec.manage(&sig_ctx));
	comp.start("hello_client", "original", Native_pd_args());
	comp.session_request_unlock();
	red_comp.start("red_client", "redundancy", Native_pd_args());
	
	/*****************************************************
	*****            start redundancy		 *****
	*****************************************************/
	
	/* just test if session_unlock() works
	 because the hello_client also for other usage, it doesn't cause any seg-fault*/

	//timer.msleep(20000);
	////red_comp.red_start();
	//red_comp.session_request_unlock();

	if (child_fault_detection(sig_rec, sig_ctx)) {
		Trace::Timestamp a = Trace::timestamp();
		printf("fault detected in monitor %lld \n", a);
		comp.child_destroy();
		red_comp.red_start();
		red_comp.session_request_unlock();

	} 
	sleep_forever();
	return 0;
}
