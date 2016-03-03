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
//#include <srv_forward.h>

/* hello interface*/
#include <hello_session/hello_session.h>
#include <failsafe_session/hello_client.h>
#include <timer_session/connection.h>

namespace Failsafe {

	using namespace Genode;

	class Hello_component;
	class Hello_root;
}


class Failsafe::Hello_component : public Rpc_object<Hello::Session>
{
	public:
		 Hello::Session_client con;
 	
		 Hello_component(Genode::Capability<Hello::Session> cap)
		 : con(cap)
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
		
		void update_cap(Genode::Capability<Hello::Session> cp)
		{
			static Hello::Session_client _con(cp);
			con  = _con;
		}

};

class Failsafe::Hello_root : public Root_component<Hello_component>
{
	private:
	
		Genode::Capability<Hello::Session> _cap;
		Hello_component* _hello;
		Genode::Lock hello_cap_barrier { Genode::Lock::LOCKED };
		
	protected:	

		Hello_component *_create_session(const char *args)
     		{
        		PDBG("creating hello session to failsafe.");
        		_hello = new (md_alloc()) Hello_component(_cap);
			//PDBG("%x", _hello);	
			hello_cap_barrier.unlock();
			return _hello;
      		}

    	public:

      		Hello_root(Genode::Rpc_entrypoint &ep,
                     	   Genode::Allocator      &allocator,
			   Genode::Capability<Hello::Session> cap)
      		: Genode::Root_component<Hello_component>(&ep, &allocator), _cap(cap)
      		{
        		PDBG("Creating root component of failsafe.");
      		}

		Hello_component* get_component()
		{
			hello_cap_barrier.lock();
			return _hello;
		}
		
};

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

	static Failsafe::Session_component red_load(size, *env()->ram_session(), cap);
	static Failsafe::Session_component load(size, *env()->ram_session(), cap);

	load.child_fault_sigh(sig_rec.manage(&sig_ctx));
        PDBG("going to start red_server");
	red_load.start("red_server", "redundancy", Native_pd_args());


        PDBG("going to start hello_server");
	load.start("hello_server", "helloser", Native_pd_args());


	load.block_for_announcement();
	static Failsafe::Hello_root hello_root(ep, *env()->heap(), load.hello_session());
	//static Failsafe::Recovery_root<Hello::Session, Failsafe::Hello_component> hello_root(ep, *env()->heap(), load.hello_session());
	env()->parent()->announce(ep.manage(&hello_root));
	

	/*****************************************************
	***** update capability when original server down*****
	*****************************************************/
	

	//while(1){
	Genode::Signal s = sig_rec.wait_for_signal();
	
	if (s.num() && s.context() == &sig_ctx) {
		PLOG("got exception for child");
		Failsafe::Hello_component* hi;
		red_load.red_start();
		red_load.block_for_announcement();
		hi = hello_root.get_component();
		hi->update_cap(red_load.hello_session());

	} else {
		PERR("got unexpected signal while waiting for child");
		class Unexpected_signal { };
		throw Unexpected_signal();
	}
	//}

        sig_rec.dissolve(&sig_ctx);

	sleep_forever();
	return 0;
}
