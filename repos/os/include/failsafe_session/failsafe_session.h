/*
 * \brief  failsafe session interface
 * \author Yusen Wang 
 * \date   2016-04-07
 */


#ifndef _INCLUDE__FAILSAFE_SESSION__FAILSAFE_SESSION_H_
#define _INCLUDE__FAILSAFE_SESSION__FAILSAFE_SESSION_H_

#include <base/rpc.h>
#include <base/rpc_args.h>
#include <dataspace/capability.h>
#include <base/signal.h>
#include <session/session.h>

namespace Failsafe {

	using Genode::Signal_context_capability;
	using Genode::Native_pd_args;

	struct Session;
}


struct Failsafe::Session : Genode::Session
{
	/*
	 * Resolve ambiguity of 'size_t' type when using 'failsafe_session.h'
	 * together with libc headers.
	 */
	typedef Genode::size_t size_t;

	/*
	 * Exception types
	 */
	struct Exception : Genode::Exception { };
	struct Rom_module_does_not_exist : Exception { };


	typedef Genode::Rpc_in_buffer<64>  Name;

	static const char *service_name() {return "Failsafe";}

	virtual void fault_sigh(Signal_context_capability sigh) = 0;

	/*******************
	 ** RPC interface **
	 *******************/



	GENODE_RPC(Rpc_fault_sigh, void, fault_sigh, Signal_context_capability);
	GENODE_RPC_INTERFACE(Rpc_fault_sigh);
	

};

#endif /* _INCLUDE__FAILSAFE_SESSION__FAILSAFE_SESSION_H_ */
