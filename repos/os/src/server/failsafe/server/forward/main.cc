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

/* Genode includes */
#include <base/env.h>
#include <base/heap.h>
#include <base/rpc_server.h>
#include <base/sleep.h>
#include <failsafe_session/failsafe_session.h>
#include <cap_session/connection.h>
#include <root/component.h>


/* local includes */
#include <child.h>
#include <ram_session_client_guard.h>
#include <rom.h>
//#include <srv_forward.h>

/* hello interface*/
#include <hello_session/hello_session.h>
#include <failsafe_session/hello_client.h>
#include <timer_session/connection.h>

namespace Failsafe {

	using namespace Genode;

	class Session_component;
	class Hello_component;
	class Root;
	class Hello_root;
}


class Failsafe::Session_component : public Rpc_object<Session> 
{
	private:

		struct Local_rom_service : Service
		{
			Rpc_entrypoint             &_ep;
			Allocator                  &_md_alloc;
			Parent_service              _parent_rom_service;
			Rom_module_registry        &_rom_modules;
			Lock                        _lock;
			List<Rom_session_component> _rom_sessions;

			void _close(Rom_session_component *rom)
			{
				_ep.dissolve(rom);
				_rom_sessions.remove(rom);
				destroy(&_md_alloc, rom);
			}

			Local_rom_service(Rpc_entrypoint      &ep,
			                  Allocator           &md_alloc,
			                  Rom_module_registry &rom_modules)
			:
				Service("virtual_rom"),
				_ep(ep),
				_md_alloc(md_alloc),
				_parent_rom_service(Rom_session::service_name()),
				_rom_modules(rom_modules)
			{ }

			~Local_rom_service()
			{
				Lock::Guard guard(_lock);

				while (_rom_sessions.first()) {
					_ep.remove(_rom_sessions.first());
					_close(_rom_sessions.first()); }
			}

			Genode::Session_capability session(char     const *args,
			                                   Affinity const &affinity)
			{
				/* try to find ROM module at local ROM service */
				try {
					Lock::Guard guard(_lock);

					char name[Session::Name::MAX_SIZE];
					
					/* extract filename from session arguments */
					Arg_string::find_arg(args, "filename")
						.string(name, sizeof(name), "");

					Rom_module &module = _rom_modules.lookup_and_lock(name);

					Rom_session_component *rom = new (&_md_alloc)
						Rom_session_component(module);

					_rom_sessions.insert(rom);

					return _ep.manage(rom);

				} catch (...) { }

				/* fall back to parent_rom_service */
				return _parent_rom_service.session(args, affinity);
			}
			
			void close(Session_capability session)
			{
				Lock::Guard guard(_lock);

				Rom_session_component *component;

				_ep.apply(session, [&] (Rom_session_component *rsc) {
					component = rsc;
					if (component) _ep.remove(component);
				});

				if (component) {
					_close(component);
					return;
				}

				_parent_rom_service.close(session);
			}

			void upgrade(Session_capability session, const char *) { }
		};

		/**
		 * Common base class of 'Local_cpu_service' and 'Local_rm_service'
		 */
		struct Intercepted_parent_service : Service
		{
			Signal_context_capability fault_sigh;

			Intercepted_parent_service(char const *name) : Service(name) { }

			void close(Session_capability session)
			{
				env()->parent()->close(session);
			}

			void upgrade(Session_capability session, const char *) { }
		};

		/**
		 * Intercept CPU session requests to install default exception
		 * handler
		 */
		struct Local_cpu_service : Intercepted_parent_service
		{
			Local_cpu_service() : Intercepted_parent_service("CPU") { }

			Genode::Session_capability session(char     const *args,
			                                   Affinity const &affinity)
			{
				Capability<Cpu_session> cap = env()->parent()->session<Cpu_session>(args, affinity);
				Cpu_session_client(cap).exception_handler(Thread_capability(), fault_sigh);
				return cap;
			}

			void upgrade(Session_capability session, const char *args)
			{
				try { env()->parent()->upgrade(session, args); }
				catch (Genode::Ipc_error)    { throw Unavailable();    }
			}
		};

		/**
		 * Intercept RM session requests to install default fault handler
		 */
		struct Local_rm_service : Intercepted_parent_service
		{
			Local_rm_service() : Intercepted_parent_service("RM") { }

			Genode::Session_capability session(char     const *args,
			                                   Affinity const &affinity)
			{
				Capability<Rm_session> cap = env()->parent()->session<Rm_session>(args, affinity);
				Rm_session_client(cap).fault_handler(fault_sigh);
				return cap;
			}
		};

		
		enum { STACK_SIZE = 2*4096 };

		size_t                    _ram_quota;
		Ram_session_client_guard  _ram_session_client;
		Heap                      _md_alloc;
		size_t                    _subsystem_ram_quota_limit;
		Rpc_entrypoint            _ep;
		Service_registry          _parent_services;
		Rom_module_registry       _rom_modules;
		Local_rom_service         _rom_service;
		Local_cpu_service         _cpu_service;
		Local_rm_service          _rm_service;
		Signal_context_capability _fault_sigh;
		Child                    *_child;

		
	public:

		/**
		 * Constructor
		 */
		Session_component(size_t quota, Ram_session &ram, Cap_session &cap)
		:
			_ram_quota(quota),
			_ram_session_client(env()->ram_session_cap(), _ram_quota),
			_md_alloc(&_ram_session_client, env()->rm_session()),
			_subsystem_ram_quota_limit(0),
			_ep(&cap, STACK_SIZE, "session_ep"),
			_rom_modules(_ram_session_client, _md_alloc),
			_rom_service(_ep, _md_alloc, _rom_modules),
			_child(0)
		{ }

		~Session_component()
		{
			if (_child)
				destroy(&_md_alloc, _child);

			/*
			 * The parent-service registry is populated by the 'Child'
			 * on demand. Revert those allocations.
			 */
			while (Service *service = _parent_services.find_by_server(0)) {
				_parent_services.remove(service);
				destroy(env()->heap(), service);
			}
		}


		/******************************
		 ** Failsafe session interface **
		 ******************************/


		
		
		void fault_sigh(Signal_context_capability sigh) override
		{
			/*
			 * CPU exception handler for CPU sessions originating from the
			 * subsystem.
			 */
			_cpu_service.fault_sigh = sigh;

			/*
			 * RM fault handler for RM sessions originating from the
			 * subsystem.
			 */
			_rm_service.fault_sigh = sigh;

			/*
			 * CPU exception and RM fault handler for the immediate child.
			 */
			_fault_sigh = sigh;
		}

		void start(Name const &binary_name, Name const &label,
		           Genode::Native_pd_args const &pd_args) 
		{
			if (_child) {
				PWRN("cannot start subsystem twice");
				return;
			}

			size_t const ram_quota = (_subsystem_ram_quota_limit > 0) ?
			                         min(_subsystem_ram_quota_limit, _ram_session_client.avail()) :
			                         _ram_session_client.avail();

			try {
				_child = new (&_md_alloc)
					Child(binary_name.string(), label.string(),
					      pd_args, _ep, _ram_session_client,
					      ram_quota, _parent_services, _rom_service,
					      _cpu_service, _rm_service, 
					      _fault_sigh);
			}
			catch (Genode::Parent::Service_denied) {
				throw Rom_module_does_not_exist(); }
		}

			
		/***************************************
		*	for child's cap
		***************************************/
		
	        Genode::Capability<Hello::Session> hello_session()
		{

			Genode::Session_capability session_cap =
				Genode::Root_client(_child->get_root_cap()).session("foo, ram_quota=4K", Genode::Affinity());

		/*
		 * The root interface returns a untyped session capability.
		 * We return a capability casted to the specific session type.
		 */
			return Genode::static_cap_cast<Hello::Session>(session_cap);
		}
		
		/************************************************
		*	blocked until child announce a service
		*************************************************/

		void block_for_announcement()
		{
			_child->block_for_hello_announcement();
		}

		/*****************************************
		*	start redundancy thread		 *
		******************************************/	

		void red_start()
		{
			_child->red_start();
		}
	
};

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
                         PDBG("pseudo say_hello finished");
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
//class Failsafe::Hello_component : public Failsafe::Recovery_component<Hello::Session, Hello::Session_client>
//{
//	public:
//		
//		void say_hello()
//                 {
//                         PDBG("pseudo say_hello");
//			 con.say_hello();
//                 }
// 
//                 int add(int a, int b)
//                 {
//                        PDBG("pseudo add");
//			return con.add(a, b);
//                 }
//
//};

class Failsafe::Root : public Root_component<Session_component>
{
	private:

		Ram_session &_ram;
		Cap_session &_cap;

	protected:

		Session_component *_create_session(const char *args)
		{
			size_t quota =
				Arg_string::find_arg(args, "ram_quota").ulong_value(0);

			return new (md_alloc()) Session_component(quota, _ram, _cap);
		}

	public:

		/**
		 * Constructor
		 *
		 * \param session_ep  entry point for managing ram session objects
		 * \param md_alloc    meta-data allocator to be used by root
		 *                    component
		 */
		Root(Rpc_entrypoint &session_ep, Allocator &md_alloc,
		     Ram_session &ram, Cap_session &cap)
		:
			Root_component<Session_component>(&session_ep, &md_alloc),
			_ram(ram), _cap(cap)
		{ 
        		PDBG("Creating root component of Failsafe.");
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

	load.fault_sigh(sig_rec.manage(&sig_ctx));
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
		ep.explicit_reply(ep.reply_dst(), 0);
                PDBG("explicit replied");
		Failsafe::Hello_component* hi;
		red_load.red_start();
		red_load.block_for_announcement();
		hi = hello_root.get_component();
		hi->update_cap(red_load.hello_session());
                PDBG("cap updated");

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