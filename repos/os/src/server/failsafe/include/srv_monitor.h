/*
 * \brief  Base Monitor class used for server monitor 
 * \author Yusen Wang
 * \date   2016-04-07
 */

#ifndef _SRV_MONITOR_H_
#define _SRV_MONITOR_H_

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

#endif /* _SRV_MONITOR_H_ */
