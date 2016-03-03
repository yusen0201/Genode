#include <root/component.h>
#include <base/rpc_server.h>
#include <base/heap.h>

namespace Failsafe {


template <typename Interface, typename Client>
class Recovery_component : public Genode::Rpc_object<Interface>
{
	protected:
		Client con;
	public:
		Recovery_component(Genode::Capability<Interface> cap)
		: con(cap)
		{}
		
		void update_cap(Genode::Capability<Interface> cp)
		{
			static Client _con(cp);
			con = _con;
		}
};

template <typename Interface, typename Recovery_comp>
class Recovery_root : public Genode::Root_component<Recovery_comp>
{
	private:
		
		Genode::Capability<Interface> _cap;
		Recovery_comp* _recovery;
		Genode::Lock cap_barrier { Genode::Lock::LOCKED };

	protected:
		
		Recovery_comp *_create_session(const char *args)
		{
			PDBG("creating session to failsafe.");
        		_recovery = new (md_alloc()) Recovery_comp(_cap);
			cap_barrier.unlock();
			return _recovery;
		}
	public:

      		Recovery_root(Genode::Rpc_entrypoint &ep,
                     	      Genode::Allocator      &allocator,
			      Genode::Capability<Interface> cap)
      		: Genode::Root_component<Recovery_comp>(&ep, &allocator), _cap(cap)
      		{
        		PDBG("Creating root component of failsafe.");
      		}

		Recovery_comp* get_component()
		{
			cap_barrier.lock();
			return _recovery;
		}
};

}
