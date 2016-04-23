/*
 * \brief  Server forwarded 
 * \author Yusen Wang
 * \date   2016-04-07
 */


#include <srv_monitor.h>
#include <srv_forward.h>

/* hello interface*/
#include <hello_session/hello_session.h>
#include <hello_session/client.h>
#include <timer_session/connection.h>

#include <base/printf.h>
#include <trace/timestamp.h>
namespace Failsafe {

	using namespace Genode;

	class Hello_sfw_monitor;
	class Hello_component;
	class Hello_root;
}

class Failsafe::Hello_sfw_monitor : public Failsafe::Srv_component<Hello::Session>
{
	public:
		Hello_sfw_monitor(size_t quota, Ram_session &ram, Cap_session &cap)
		: Srv_component(quota, ram, cap)
		{}
	
		Genode::Capability<Hello::Session> child_session() override
		{
	
			Genode::Session_capability session_cap =
				Genode::Root_client(_child->get_root_cap()).session("foo, ram_quota=4K", Genode::Affinity());

			/*
		 	* The root interface returns a untyped session capability.
		 	* We return a capability casted to the specific session type.
		 	*/
			return Genode::static_cap_cast<Hello::Session>(session_cap);
	
		}	
};

class Failsafe::Hello_component : public Failsafe::Recovery_component<Hello::Session, Hello::Session_client>
{
	public:
		Trace::Timestamp t;		

		void say_hello()
                 {
			t = Trace::timestamp();
			printf("in pseudo server %u \n", t);
                        PDBG("pseudo say_hello");
			try {
				if(!constructed)
					construct_barrier.down();
			t = Trace::timestamp();
			printf("pseudo server start say_hello %u \n", t);
				con->say_hello();
			t = Trace::timestamp();
			printf("pseudo server finish say_hello %u \n", t);
				} 
			catch(Genode::Ipc_error) {
                        	PDBG("say_hello ipc exception");
				construct_barrier.down();
			 	con->say_hello();
				}
			catch(Genode::Blocking_canceled) {
                        	PDBG("say_hello blk exception");
				construct_barrier.down();
			 	con->say_hello();
				}

                 }
 
                 int add(int a, int b)
                 {
                        PDBG("pseudo add");
			try { 
				if(!constructed)
					construct_barrier.down();
				return con->add(a, b); 
				}
			catch(Genode::Ipc_error) { 
                        	PDBG("add ipc exception");
				construct_barrier.down();
				return con->add(a, b); 
				}
			catch(Genode::Blocking_canceled) { 
                        	PDBG("add blk exception");
				construct_barrier.down();
				return con->add(a, b); 
				}
                 }

};



int main()
{
	using namespace Genode;
	
	Timer::Connection timer;
	size_t size=10*1024*1024;
	enum { STACK_SIZE = 8*1024 };
	static Cap_connection cap;
	static Rpc_entrypoint ep(&cap, STACK_SIZE, "failsafe_ep");

	static Signal_receiver sig_rec;
	static Signal_receiver red_sig_rec;
	Signal_context *sig_ctx = new (env()->heap()) Signal_context;
	Signal_context *red_sig_ctx = new (env()->heap()) Signal_context;

	Failsafe::Hello_sfw_monitor *red_comp = new (env()->heap()) Failsafe::Hello_sfw_monitor(size, *env()->ram_session(), cap);
	Failsafe::Hello_sfw_monitor *comp = new (env()->heap()) Failsafe::Hello_sfw_monitor(size, *env()->ram_session(), cap);
	
	static Failsafe::Recovery_root<Hello::Session, Failsafe::Hello_component> 
			hello_root(ep, *env()->heap());
	env()->parent()->announce(ep.manage(&hello_root));

	Failsafe::Hello_component* hi;
	hi = hello_root.get_component();


	comp->child_fault_sigh(sig_rec.manage(&(*sig_ctx)));
	red_comp->child_fault_sigh(red_sig_rec.manage(&(*red_sig_ctx)));

        PDBG("going to start hello_server");
	comp->start("hello_server", "helloser", Native_pd_args());

        PDBG("going to start red_server");
	red_comp->start("red_server", "redundanc", Native_pd_args());

	comp->block_for_announcement();
	hi->construct(comp->child_session());

	
	/*****************************************************
	***** update capability when original server down*****
	*****************************************************/
	
	int tag = 0;
	
	while(1){
	
	switch(tag) {
	case 0:
	if (child_fault_detection(sig_rec, *sig_ctx)) {
		Trace::Timestamp a = Trace::timestamp();
		printf("fault detected in monitor %u \n", a);
        	sig_rec.dissolve(&(*sig_ctx));
        	tag++;
		comp->child_destroy();	

		/* activate redundancy */
		red_comp->block_for_announcement();
		hi->construct(red_comp->child_session());
		
		/* recreate a child */
			
		//static Failsafe::Hello_sfw_monitor re_comp(size, *env()->ram_session(), cap);
        	//PDBG("going to recreate child");
		//re_comp.start("red_server", "rechild", Native_pd_args());
		//re_comp.block_for_announcement();
		//hi->construct(re_comp.child_session());
		PDBG("to recreate original");
		comp = new (env()->heap()) Failsafe::Hello_sfw_monitor(size, *env()->ram_session(), cap);	
		sig_ctx = new (env()->heap()) Signal_context;
		comp->child_fault_sigh(sig_rec.manage(&(*sig_ctx)));
		comp->start("hello_server", "helloser", Native_pd_args());
	
		}
	break;

	case 1:
	if (child_fault_detection(red_sig_rec, *red_sig_ctx)) {
			PDBG("red fault");
        		red_sig_rec.dissolve(&(*red_sig_ctx));
			tag--;
			red_comp->child_destroy();
			
			/* activate original */	
			comp->block_for_announcement();
			hi->construct(comp->child_session());

			PDBG("to recreate redundancy");
			red_comp = new (env()->heap()) Failsafe::Hello_sfw_monitor(size, *env()->ram_session(), cap);	
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
