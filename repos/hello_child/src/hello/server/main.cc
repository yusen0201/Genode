/*
 * \brief  Main program of the Hello server
 * \author Björn Döbel
 * \date   2008-03-20
 */

/*
 * Copyright (C) 2008-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/printf.h>
#include <base/env.h>
#include <base/sleep.h>
#include <base/child.h>
#include <pd_session/connection.h>
#include <cpu_session/connection.h>
#include <rom_session/connection.h>
#include <rm_session/connection.h>
#include <ram_session/connection.h>
#include <cap_session/connection.h>
#include <root/component.h>
#include <hello_session.h>
#include <base/rpc_server.h>
#include <timer_session/connection.h>

namespace Hello {

	struct Session_component : Genode::Rpc_object<Session>
	{
		void say_hello() {
			PDBG("I am here... Hello."); }

		int add(int a, int b) {
			return a + b; }
	};

	class Root_component : public Genode::Root_component<Session_component>
	{
		protected:

			Hello::Session_component *_create_session(const char *args)
			{
				PDBG("creating hello session.");
				return new (md_alloc()) Session_component();
			}

		public:

			Root_component(Genode::Rpc_entrypoint *ep,
			               Genode::Allocator *allocator)
			: Genode::Root_component<Session_component>(ep, allocator)
			{
				PDBG("Creating root component.");
			}
	};
}

static void wait_for_signal_for_context(Genode::Signal_receiver &sig_rec,
                                        Genode::Signal_context const &sig_ctx)
{
	Genode::Signal s = sig_rec.wait_for_signal();

	if (s.num() && s.context() == &sig_ctx) {
		PLOG("got exception for child");
		
	} else {
		PERR("got unexpected signal while waiting for child");
		class Unexpected_signal { };
		throw Unexpected_signal();
	}
}

/******************************************************************
 ** Test for detecting the failure of an immediate child process **
 ******************************************************************/

class Test_child : public Genode::Child_policy
{
	private:

		struct Resources
		{
			Genode::Pd_connection  pd;
			Genode::Ram_connection ram;
			Genode::Cpu_connection cpu;
			Genode::Rm_connection  rm;

			Resources(Genode::Signal_context_capability sigh, char const *label)
			: pd(label)
			{
				using namespace Genode;

				/* transfer some of our own ram quota to the new child */
				enum { CHILD_QUOTA = 1*1024*1024 };
				ram.ref_account(env()->ram_session_cap());
				env()->ram_session()->transfer_quota(ram.cap(), CHILD_QUOTA);

				/* register default exception handler */
				cpu.exception_handler(Thread_capability(), sigh);

				/* register handler for unresolvable page faults */
				rm.fault_handler(sigh);
			}
		} _resources;

		/*
		 * The order of the following members is important. The services must
		 * appear before the child to ensure the correct order of destruction.
		 * I.e., the services must remain alive until the child has stopped
		 * executing. Otherwise, the child may hand out already destructed
		 * local services when dispatching an incoming session call.
		 */
		Genode::Rom_connection _elf;
		Genode::Parent_service _log_service;
		Genode::Parent_service _rm_service;
		Genode::Parent_service _signal;
		Genode::Parent_service _timer_service;
		Genode::Local_service  _hello_service;
		Genode::Child          _child;

	public:

		/**
		 * Constructor
		 */
		Test_child(Genode::Rpc_entrypoint           &ep,
		           char const                       *elf_name,
		           Genode::Signal_context_capability sigh,
			   Hello::Root_component &root)
		:
			_resources(sigh, elf_name),
			_elf(elf_name),
			_log_service("LOG"), _rm_service("RM"), _signal("SIGNAL"), _timer_service("Timer"), _hello_service("Hello", &root),
			_child(_elf.dataspace(), _resources.pd.cap(), _resources.ram.cap(),
			       _resources.cpu.cap(), _resources.rm.cap(), &ep, this)
		{ PLOG("create child ");}


		/****************************
		 ** Child-policy interface **
		 ****************************/

		const char *name() const { return "child"; }

		Genode::Service *resolve_session_request(const char *service, const char *)
		{
			/* forward white-listed session requests to our parent */
			if(!Genode::strcmp(service, "Hello"))
				return &_hello_service;
			return !Genode::strcmp(service, "LOG") ? &_log_service
			     : !Genode::strcmp(service, "RM")  ? &_rm_service
			     : !Genode::strcmp(service, "Timer")  ? &_timer_service
			     : !Genode::strcmp(service, "SIGNAL")  ? &_signal
			     : 0;
		}

		void filter_session_args(const char *service,
		                         char *args, Genode::size_t args_len)
		{
			/* define session label for sessions forwarded to our parent */
			Genode::Arg_string::set_arg(args, args_len, "label", "child");
		} 

};

void Create_child(Genode::Rpc_entrypoint           &ep,
		  char const                       *elf_name,
		  Genode::Signal_context_capability sigh,
		  Hello::Root_component &root)
{
      static  Test_child child(ep, elf_name, sigh, root);
}

using namespace Genode;

int main(int argc, char **argv)
{
	using namespace Genode;
	/*
	 * Get a session for the parent's capability service, so that we
	 * are able to create capabilities.
	 */
	Cap_connection cap;

	/*
	 * A sliced heap is used for allocating session objects - thereby we
	 * can release objects separately.
	 */
	static Sliced_heap sliced_heap(env()->ram_session(),
	                               env()->rm_session());


	enum { STACK_SIZE = 10*1024 };
	static Rpc_entrypoint ep(&cap, STACK_SIZE, "hello_ep");

	Cap_connection cap2;
	Rpc_entrypoint ep2(&cap2, STACK_SIZE, "child");	

	static Hello::Root_component hello_root(&ep, &sliced_heap);
	env()->parent()->announce(ep.manage(&hello_root));
	
    Signal_receiver sig_rec;
    Signal_context  sig_ctx;

    Signal_receiver sig_recr;
    Signal_context  sig_ctxr;
    /*
     * Iteratively start a faulting program and detect the faults
     */

	Timer::Connection timer;
        //PLOG("create child ");

        /* create and start child process */
        Test_child child(ep, "test_hello", sig_rec.manage(&sig_ctx), hello_root);

        PLOG("wait_for_signal");

	Genode::Signal s = sig_rec.wait_for_signal();
 
	if (s.num() && s.context() == &sig_ctx) {
		PLOG("got exception for child");
        //	sig_rec.dissolve(&sig_ctx);
	static	Test_child childr(ep, "test_r", sig_rec.manage(&sig_ctxr), hello_root);		
	} else {
		PERR("got unexpected signal while waiting for child");
		class Unexpected_signal { };
		throw Unexpected_signal();
	}

        sig_rec.dissolve(&sig_ctx);
      //  faulting_child_test();
	/* We are done with this and only act upon client requests now. */
	sleep_forever();


	return 0;
}
