#include <child.h>
#include <base/rpc_client.h>
#include <root/client.h>
#include <hello_session/hello_session.h>

namespace Hello {

	struct Hello_cap : Loader::Child
	{
		
		Genode::Root_capability log_root_cap;

	        Genode::Capability<Hello::Session> hello_session()
		{

			Genode::Session_capability session_cap =
				Genode::Root_client(log_root_cap).session("foo, ram_quota=4K", Genode::Affinity());

		/*
		 * The root interface returns a untyped session capability.
		 * We return a capability casted to the specific session type.
		 */
			return Genode::static_cap_cast<Hello::Session>(session_cap);
		}

		bool announce_service(const char             *service_name,
	                      Genode::Root_capability root_cap,
	                      Genode::Allocator      *,
	                      Genode::Server         *) override
		{

		if (Genode::strcmp("Hello", service_name) == 0) 
			log_root_cap = root_cap;

		return false;
		}
	};

}
