#ifndef _MONITOR_H_
#define _MONITOR_H_

/* Genode includes */
#include <base/env.h>
#include <base/heap.h>
#include <base/rpc_server.h>
#include <base/sleep.h>
#include <failsafe_session/failsafe_session.h>
#include <cap_session/connection.h>
#include <root/component.h>


/* local includes */
#include <m_child.h>
#include <ram_session_client_guard.h>
#include <rom.h>


namespace Failsafe {

	using namespace Genode;
	
	class Session_component;
	class Root;
}


class Failsafe::Session_component : public Rpc_object<Session> 
{
	protected:
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
		Child                     *_child;

		
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
	
		/**
		 * fault_sigh for subsystem
		 */

		void child_fault_sigh(Signal_context_capability sigh) 
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

		/**
		 * create child 
		 */

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

		/*
		 **	get child's root cap
		 */
		Root_capability child_root_cap()
		{
			return _child->get_root_cap();
		}
		
			
		/*
		 ** blocked until child announce a service
		 */

		void block_for_announcement()
		{
			_child->block_for_child_announcement();
		}

		/*
		 ** unlock session_resolve_request 
		 */

		void session_request_unlock()
		{
			_child->session_request_unlock();
		}

		/*
		 ** start redundancy thread		 
		 */	

		void red_start()
		{
			_child->red_start();
		}
	
		/******************************
		 ** Failsafe session interface **
		 ******************************/

		Signal_transmitter cli_sig;
		
		void fault_sigh(Signal_context_capability sigh) override
		{
			cli_sig.context(sigh);
		}
};

class Failsafe::Root : public Root_component<Session_component>
{
	private:

		Ram_session &_ram;
		Cap_session &_cap;
		Session_component* _root;
		Genode::Lock loader_cap_barrier { Genode::Lock::LOCKED };

	protected:

		Session_component *_create_session(const char *args)
		{
        		PDBG("Creating loader session of Failsafe.");
			size_t quota =
				Arg_string::find_arg(args, "ram_quota").ulong_value(0);

			_root = new (md_alloc()) Session_component(quota, _ram, _cap);

			loader_cap_barrier.unlock();
			return _root;
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
		

		Session_component* get_component()
		{
			loader_cap_barrier.lock();
			return _root;	
		}
};


/*********************************************************************
 **************           Fault detection            *****************
 ********************************************************************/

bool child_fault_detection(Genode::Signal_receiver &sig_rec,
                           Genode::Signal_context const &sig_ctx)
{
	Genode::Signal s = sig_rec.wait_for_signal();

	if (s.num() && s.context() == &sig_ctx) {
		PLOG("got exception for child");
		return true;
	} else {
		PERR("got unexpected signal while waiting for child");
		class Unexpected_signal { };
		throw Unexpected_signal();
	}
}

#endif /* _MONITOR_H_ */
