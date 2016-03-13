#include <root/component.h>
#include <base/rpc_server.h>
#include <util/volatile_object.h>

namespace Failsafe {

using Genode::Lazy_volatile_object;

template <typename Interface, typename Client>
class Recovery_component : public Genode::Rpc_object<Interface>
{
	protected:
		Lazy_volatile_object<Client> con;
		bool constructed;
		Genode::Semaphore construct_barrier;
	public:
		
		Recovery_component()
		: constructed(false) 
		{}

		void construct(Genode::Capability<Interface> cp)
		{
			con.construct(cp);
			construct_barrier.up();
			constructed = true;
		}
};

template <typename Interface, typename Recovery_comp>
class Recovery_root : public Genode::Root_component<Recovery_comp>
{
	private:
		
		Genode::Capability<Interface> _cap;
		Recovery_comp* _recovery;
		Genode::Semaphore comp_barrier;

	protected:
		
		Recovery_comp *_create_session(const char *args)
		{
			PDBG("creating session to failsafe.");
        		_recovery = new (this->md_alloc()) Recovery_comp;
			comp_barrier.up();
			return _recovery;
		}

	public:

		Recovery_root(Genode::Rpc_entrypoint &ep,
                     	      Genode::Allocator      &allocator)
      		: Genode::Root_component<Recovery_comp>(&ep, &allocator)
      		{
        		PDBG("Creating root component of failsafe.");
      		}
		Recovery_comp* get_component()
		{
			comp_barrier.down();
			return _recovery;
		}
};

}
