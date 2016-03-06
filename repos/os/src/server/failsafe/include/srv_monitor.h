#ifndef _SFW_MONITOR_H_
#define _SFW_MONITOR_H_

#include <monitor.h>

namespace Failsafe {
	using namespace Genode;
	
	template <typename Interface>
	class Srv_component;	
}

template <typename Interface>
class Failsafe::Srv_component : public Failsafe::Session_component
{
	public:
		/**
		 * session capability from child 
	 	 */
		
		Srv_component(size_t quota, Ram_session &ram, Cap_session &cap)
		: Session_component(quota, ram, cap)
		{}
		
		/**
		 * call the child and returns session capability
		 */

	        virtual Genode::Capability<Interface> child_session() = 0;
};

#endif /* _SFW_MONITOR_H_ */
