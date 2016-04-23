/*
 * \brief  Client monitor used for monitor for both client and server solution 
 * \author Yusen Wang
 * \date   2016-04-07
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
	Signal_context *sig_ctx_srv = new (env()->heap()) Signal_context;

	static Failsafe::Connection srv_monitor(1024*1024);
	srv_monitor.fault_sigh(sig_rec.manage(&(*sig_ctx_srv)));

	Failsafe::Session_component *red_comp = new (env()->heap()) Failsafe::Session_component(size, *env()->ram_session(), cap);
	Failsafe::Session_component *comp = new (env()->heap()) Failsafe::Session_component(size, *env()->ram_session(), cap);

	comp->start("hello_client", "", Native_pd_args());
	comp->session_request_unlock();
	red_comp->start("red_client", "red", Native_pd_args()); //block thread


	/*****************************************************
	*****            start redundancy		 *****
	*****************************************************/
	int tag = 0;

	while(1) {	

	if (child_fault_detection(sig_rec, *sig_ctx_srv)) {
		Trace::Timestamp a = Trace::timestamp();
		printf("fault detected in monitor: %d \n", a);
        	sig_rec.dissolve(&(*sig_ctx_srv));
		sig_ctx_srv = new (env()->heap()) Signal_context;
		srv_monitor.fault_sigh(sig_rec.manage(&(*sig_ctx_srv)));

		if (tag == 0) {
        		tag++;
			comp->child_destroy();

			/* redundancy */
			PLOG("got signal from server monitor");
			//red_comp.red_start(); //block thread	
			red_comp->session_request_unlock();
			PLOG("redundancy started");
			
			/* recreate a child */
			//PDBG("going to recreate child");
			//static Failsafe::Session_component comp(size, *env()->ram_session(), cap);
			//comp.start("red_client", "rechild", Native_pd_args());
			//comp.session_request_unlock();
	
			//sig_rec.dissolve(&sig_ctx_srv);
			PDBG("to recreate original");
			comp = new (env()->heap()) Failsafe::Session_component(size, *env()->ram_session(), cap);	
			comp->start("hello_server", "helloser", Native_pd_args());
		}
		else if (tag == 1) {
        		tag--;
			red_comp->child_destroy();
			
			/* activate original */
			comp->session_request_unlock();
	
			PDBG("to recreate redundancy");
			red_comp = new (env()->heap()) Failsafe::Session_component(size, *env()->ram_session(), cap);	
			red_comp->start("red_server", "helloser", Native_pd_args());
		}
	} 

	}

	sleep_forever();
	return 0;
}
