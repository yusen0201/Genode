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
#include <srv_monitor.h>
#include <srv_forward.h>

/* hello interface*/
#include <hello_session/hello_session.h>
#include <failsafe_session/hello_client.h>
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
		

		void say_hello()
                 {
                        PDBG("pseudo say_hello");
			try {
				if(!constructed)
					construct_barrier.down();
				con->say_hello();
				} 
			catch(Genode::Ipc_error) {
                        PDBG("say_hello exception");
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
                        PDBG("add exception");
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
	Signal_context sig_ctx;
	Signal_context red_sig_ctx;

	static Failsafe::Hello_sfw_monitor red_comp(size, *env()->ram_session(), cap);
	static Failsafe::Hello_sfw_monitor comp(size, *env()->ram_session(), cap);
	
	static Failsafe::Recovery_root<Hello::Session, Failsafe::Hello_component> 
			hello_root(ep, *env()->heap());
	env()->parent()->announce(ep.manage(&hello_root));

	Failsafe::Hello_component* hi;
	hi = hello_root.get_component();


	comp.child_fault_sigh(sig_rec.manage(&sig_ctx));
	red_comp.child_fault_sigh(red_sig_rec.manage(&red_sig_ctx));

        PDBG("going to start hello_server");
	comp.start("hello_server", "helloser", Native_pd_args());

        PDBG("going to start red_server");
	red_comp.start("red_server", "redundanc", Native_pd_args());

	comp.block_for_announcement();
	hi->construct(comp.child_session());

	
	/*****************************************************
	***** update capability when original server down*****
	*****************************************************/
	//Failsafe::Child *childa, *childb;
	//childa = comp.child();
	//childb = red_comp.child();
	//PDBG("a %x", childa);
	//PDBG("b %x", childb);
	
	int tag = 0;
	
	while(1){
	
	switch(tag) {
	case 0:
	if (child_fault_detection(sig_rec, sig_ctx)) {
		Trace::Timestamp a = Trace::timestamp();
		printf("fault detected in monitor %lld \n", a);
        	//sig_rec.dissolve(&sig_ctx);
		comp.child_destroy();	
		//comp.reset_child();
		//childa = comp.child();
		//PDBG("a after fault %x", childa);
		//Failsafe::Hello_component* hi;
		//red_comp.red_start();
		red_comp.block_for_announcement();
		//hi = hello_root.get_component();
		hi->construct(red_comp.child_session());
        	tag++;
		//comp.fast_restart();
		//PDBG("11");
		//comp.block_for_announcement();
		//PDBG("11");
		
		//hi->construct(comp.child_session());
		//comp.start("hello_server", "srv_recreate", Native_pd_args());
		//PDBG("fault, recreate hello_server");
		}
	break;

	case 1:
	if (child_fault_detection(red_sig_rec, red_sig_ctx)) {
			PDBG("red fault");
			red_comp.child_destroy();
			//comp.block_for_announcement();
			hi->construct(comp.child_session());
			tag--;
			//red_comp.fast_restart();	
		//Failsafe::Hello_component* hi;
		//hi = hello_root.get_component();
		//hi->update_cap(red_comp.child_session());
			//PDBG("restarted");
        	//sig_rec.dissolve(&red_sig_ctx);
		}
	break;
	}
	}
	

        sig_rec.dissolve(&sig_ctx);

	sleep_forever();
	return 0;
}
