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
		
		Hello_component(Genode::Capability<Hello::Session> cap)
		: Recovery_component<Hello::Session, Hello::Session_client>(cap)
		{}

		void say_hello()
                 {
                         PDBG("pseudo say_hello");
			 con.say_hello();
                 }
 
                 int add(int a, int b)
                 {
                        PDBG("pseudo add");
			return con.add(a, b);
                 }

};



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

	static Failsafe::Hello_sfw_monitor red_comp(size, *env()->ram_session(), cap);
	static Failsafe::Hello_sfw_monitor comp(size, *env()->ram_session(), cap);

	comp.child_fault_sigh(sig_rec.manage(&sig_ctx));

        PDBG("going to start hello_server");
	comp.start("hello_server", "helloser", Native_pd_args());

        PDBG("going to start red_server");
	red_comp.start("red_server", "redundancy", Native_pd_args());

	comp.block_for_announcement();

	static Failsafe::Recovery_root<Hello::Session, Failsafe::Hello_component> 
			hello_root(ep, *env()->heap(), comp.child_session());

	env()->parent()->announce(ep.manage(&hello_root));
	

	/*****************************************************
	***** update capability when original server down*****
	*****************************************************/
	

	while(1){
	//Genode::Signal s = sig_rec.wait_for_signal();
	
	if (child_fault_detection(sig_rec, sig_ctx)) {
		ep.explicit_reply(ep.reply_dst(), 1);
		Failsafe::Hello_component* hi;
		red_comp.red_start();
		red_comp.block_for_announcement();
		hi = hello_root.get_component();
		hi->update_cap(red_comp.child_session());

		} 
	}

        sig_rec.dissolve(&sig_ctx);

	sleep_forever();
	return 0;
}
