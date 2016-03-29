/*
 * \brief  Failsafe service
 * \author Norman Feske
 * \date   2012-04-17
 */

/*
 * Copyright (C) 2010-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */


#include <srv_monitor.h>

#include <hello_session/hello_session.h>
#include <timer_session/connection.h>


#include <trace/timestamp.h>
#include <base/printf.h>

int main()
{
	using namespace Genode;
	
	Timer::Connection timer;
	size_t size=1024*1024;
	enum { STACK_SIZE = 8*1024 };
	static Cap_connection cap;
	static Rpc_entrypoint ep(&cap, STACK_SIZE, "failsafe_ep");
	static Signal_receiver sig_rec;
	Signal_context sig_ctx;

	static Failsafe::Session_component red_comp(size, *env()->ram_session(), cap);
	static Failsafe::Session_component comp(size, *env()->ram_session(), cap);

	static Failsafe::Root root(ep, *env()->heap(), *env()->ram_session(), cap);

	env()->parent()->announce(ep.manage(&root));

	comp.child_fault_sigh(sig_rec.manage(&sig_ctx));
	comp.start("hello_server", "", Native_pd_args());
	red_comp.start("red_server", "", Native_pd_args());
	
	comp.block_for_announcement();
	env()->parent()->announce("Hello", comp.child_root_cap());


	static Failsafe::Session_component* comp_component;
	comp_component = root.get_component();



	/*****************************************************
	***** update capability when original server down*****
	*****************************************************/
	


	
	if (child_fault_detection(sig_rec, sig_ctx)) {
		Trace::Timestamp a = Trace::timestamp();
		printf("fault detected: %u \n", a);
		comp.child_destroy();
		
		/* redundancy */
		red_comp.block_for_announcement();
		env()->parent()->announce("Hello", red_comp.child_root_cap());
		
		/* recreate a child */
		//static Failsafe::Session_component re_comp(size, *env()->ram_session(), cap);
        	//PDBG("going to recreate child");
		//re_comp.start("red_server", "rechild", Native_pd_args());
		//re_comp.block_for_announcement();
		//env()->parent()->announce("Hello", re_comp.child_root_cap());

		PLOG("send signal");
		comp_component->cli_sig.submit();
		PLOG("signal sant");

	} 

	sleep_forever();
	return 0;
}
