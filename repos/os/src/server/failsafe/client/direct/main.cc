/*
 * \brief  Client direct 
 * \author Yusen Wang
 * \date   2016-04-07
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
	static Signal_receiver red_sig_rec;
	Signal_context *sig_ctx = new (env()->heap()) Signal_context;
	Signal_context *red_sig_ctx = new (env()->heap()) Signal_context;

	Failsafe::Session_component *red_comp = new (env()->heap()) Failsafe::Session_component(size, *env()->ram_session(), cap);
	Failsafe::Session_component *comp = new (env()->heap()) Failsafe::Session_component(size, *env()->ram_session(), cap);

	comp->child_fault_sigh(sig_rec.manage(&(*sig_ctx)));
	comp->start("hello_client", "original", Native_pd_args());
	comp->session_request_unlock();
	red_comp->child_fault_sigh(red_sig_rec.manage(&(*red_sig_ctx)));
	red_comp->start("red_client", "red", Native_pd_args()); //block thread
	
	/*****************************************************
	*****            start redundancy		 *****
	*****************************************************/

	int tag = 0;
	
	while(1) {
	switch(tag) {
	case 0:
	if (child_fault_detection(sig_rec, *sig_ctx)) {
		Trace::Timestamp a = Trace::timestamp();
		printf("fault detected in monitor %u \n", a);
        	sig_rec.dissolve(&(*sig_ctx));
        	tag++;
		comp->child_destroy();

		/* activate redundancy */
		//red_comp->red_start(); //block thread
		red_comp->session_request_unlock();

		/* recreate a child */
		//PDBG("going to recreate child");
		//static Failsafe::Session_component re_comp(size, *env()->ram_session(), cap);
		//re_comp.start("red_client", "rechild", Native_pd_args());
		//re_comp.session_request_unlock();
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
		comp->session_request_unlock();
	
		PDBG("to recreate redundancy");
		red_comp = new (env()->heap()) Failsafe::Session_component(size, *env()->ram_session(), cap);	
		red_sig_ctx = new (env()->heap()) Signal_context;
		red_comp->child_fault_sigh(red_sig_rec.manage(&(*red_sig_ctx)));
		red_comp->start("red_server", "helloser", Native_pd_args());
	}
	}
	}
	sleep_forever();
	return 0;
}
