/*
 * \brief  Server direct 
 * \author Yusen Wang
 * \date   2016-04-07
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
	static Signal_receiver red_sig_rec;
	Signal_context *sig_ctx = new (env()->heap()) Signal_context;
	Signal_context *red_sig_ctx = new (env()->heap()) Signal_context;

	Failsafe::Session_component *red_comp = new (env()->heap()) Failsafe::Session_component(size, *env()->ram_session(), cap);
	Failsafe::Session_component *comp = new (env()->heap()) Failsafe::Session_component(size, *env()->ram_session(), cap);

	static Failsafe::Root root(ep, *env()->heap(), *env()->ram_session(), cap);

	env()->parent()->announce(ep.manage(&root));

	comp->child_fault_sigh(sig_rec.manage(&(*sig_ctx)));
	comp->start("hello_server", "helloser", Native_pd_args());
	red_comp->child_fault_sigh(red_sig_rec.manage(&(*red_sig_ctx)));
	red_comp->start("red_server", "redundanc", Native_pd_args());
	
	comp->block_for_announcement();
	env()->parent()->announce("Hello", comp->child_root_cap());


	static Failsafe::Session_component* comp_component;
	comp_component = root.get_component();



	/*****************************************************
	***** update capability when original server down*****
	*****************************************************/
	int tag = 0;
	
	while(1){	

	switch(tag) {
	case 0:
	if (child_fault_detection(sig_rec, *sig_ctx)) {
        	sig_rec.dissolve(&(*sig_ctx));
        	tag++;
		comp->child_destroy();
		
		/* activate redundancy */
		red_comp->block_for_announcement();
		env()->parent()->announce("Hello", red_comp->child_root_cap());
		
		/* recreate a child */
		//static failsafe::session_component re_comp(size, *env()->ram_session(), cap);
        	//pdbg("going to recreate child");
		//re_comp.start("red_server", "rechild", native_pd_args());
		//re_comp.block_for_announcement();
		//env()->parent()->announce("hello", re_comp.child_root_cap());

		PDBG("send signal");
		comp_component->cli_sig.submit();
		PDBG("signal sant");

		PDBG("to recreate original");
		comp = new (env()->heap()) Failsafe::Session_component(size, *env()->ram_session(), cap);	
		sig_ctx = new (env()->heap()) Signal_context;
		comp->child_fault_sigh(sig_rec.manage(&(*sig_ctx)));
		comp->start("hello_server", "helloser", Native_pd_args());

	}
	break;

	case 1:
	if (child_fault_detection(red_sig_rec, *red_sig_ctx)) {
        	red_sig_rec.dissolve(&(*red_sig_ctx));
        	tag--;
		red_comp->child_destroy();
		
		/* activate original */
		comp->block_for_announcement();
		env()->parent()->announce("hello", comp->child_root_cap());
		
		/* recreate a child */
		//static failsafe::session_component re_comp(size, *env()->ram_session(), cap);
        	//pdbg("going to recreate child");
		//re_comp.start("red_server", "rechild", native_pd_args());
		//re_comp.block_for_announcement();
		//env()->parent()->announce("hello", re_comp.child_root_cap());

		PDBG("send signal");
		comp_component->cli_sig.submit();
		PDBG("signal sant");
		
		PDBG("to recreate redundancy");
		red_comp = new (env()->heap()) Failsafe::Session_component(size, *env()->ram_session(), cap);	
		red_sig_ctx = new (env()->heap()) Signal_context;
		red_comp->child_fault_sigh(red_sig_rec.manage(&(*red_sig_ctx)));
		red_comp->start("red_server", "helloser", Native_pd_args());

	}
	break;

	}
	}

	sleep_forever();
	return 0;
}
